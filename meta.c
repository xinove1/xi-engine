#include <stdio.h>
#include <stdlib.h>
#define NO_RAYLIB
#include "./src/modules/core.h"
#include "./nob.h"

// NOTE  no checking of limit when using this, only 2 places tho
global byte scratch_buf[10000] = {0};
global size scratch_buf_at = 0;

typedef enum {
	TokenOpenParan,
	TokenCloseParan,
	TokenColon,
	TokenSemiColon,
	TokenOpenBracket,
	TokenCloseBracket,
	TokenOpenBraces,
	TokenCloseBraces,
	TokenAsterisk,

	// TokenStruct,
	// TokenUnion,
	TokenString,
	TokenIndentifier,
	TokenUnknown,

	TokenEOF,
	TokenCount,
} TokenType;

global cstr *TokenTypeNames[] = {
	"TokenOpenParan",
	"TokenCloseParan",
	"TokenColon",
	"TokenSemiColon",
	"TokenOpenBracket",
	"TokenCloseBracket",
	"TokenOpenBraces",
	"TokenCloseBraces",
	"TokenAsterisk",
	"TokenString",
	"TokenIndentifier",
	"TokenUnknown",
	"TokenEOF",
	"TokenCount",
};

typedef struct {
	TokenType type;
	cstr *text;
	i32 text_size;
} Token;

typedef struct {
	cstr *at;
} Tokenizer;

typedef struct {
	cstr **items;
	size count;
	size capacity;
} cstrDa;

typedef enum {FieldNormal, FieldIsPointer, FieldIsArray, FieldFlagCount} MetaMemberFlags ;

typedef  struct {
	MetaMemberFlags flags;
	cstr *type;
	cstr *name;
	cstr *arr_name; // Name of indentifier used to define arr_size, aka macros
	size arr_size;
} MetaMember;

typedef struct {
	cstr *name;
	cstrDa fields;
	cstrDa aliases; // Extra info for use in structs that have a enum as type etc etc
} MetaEnum;

typedef struct {
	MetaMember *items;
	size count;
	size capacity;
} MetaMemberDa;

typedef struct {
	cstr *name;
	MetaMemberDa fields;
} MetaStruct;

typedef struct {
	MetaStruct *items;
	size count;
	size capacity;
} MetaStructDa;

typedef struct {
	MetaStruct meta_struct;
	MetaMember *union_tag;
	MetaStructDa union_structs;
} MetaStructTagged;

typedef enum {I_NONE, I_Struct, I_StructTaggedUnion, I_Enum, I_Count} IntrospectableTypes;

const global cstr *IntrospectableTypesNames[] = {
	"I_NONE", "I_Struct", "I_StructTaggedUnion", "I_Enum", "I_Count",
};

typedef struct {
	IntrospectableTypes type;
	union {
		MetaStruct meta_struct;
		MetaStructTagged meta_struct_tagged;
		MetaEnum meta_enum;
	 };
} Introspectable;

typedef struct {
	Introspectable *items;
	size count;
	size capacity;
} IntrospectableDa;

global cstrDa EncounteredTypes = {0};


void print_introspectable(IntrospectableDa *da);
internal void print_member(MetaMember m, size depth);
internal void print_struct(MetaStruct s, size depth);
internal void print_Enum(MetaEnum e, size depth);

internal cstr *read_file(cstr *path);
internal cstr *copy_token_text(Token token) ;
internal Token get_token(Tokenizer *t);
internal cstr *get_type(Token token);
internal void parse_file(cstr *path, IntrospectableDa *introspectables);

internal Introspectable parse_instrospectable(Tokenizer *t);
internal IntrospectableTypes parse_instrospectable_params(Tokenizer *t);
internal MetaEnum parse_enum(Tokenizer *t);
internal MetaStruct parse_struct(Tokenizer *t);
internal MetaStruct parse_struct_union_struct(Tokenizer *t);
internal MetaStructTagged parse_struct_tagged(Tokenizer *t);
internal MetaMember parse_struct_member(Tokenizer *t, Token member_token);
internal MetaStructDa parse_struct_union(Tokenizer *t);
internal b32 token_match(Token t, cstr *str);
internal b32 has_token(Tokenizer *t, TokenType token_type);

internal void generate_output(cstr *path, IntrospectableDa *data);
internal void gen_struct(FILE *f, MetaStruct s);
internal void gen_struct_tagged(FILE *f, MetaStruct s);

void introspect_files(cstr *output_file, cstr **files, size files_count) 
{
	// TODO  Check files against generated code to see if we actually need to process them
	
	IntrospectableDa info = {0};

	for (i32 i = 0; i < files_count; i++) {
		nob_log(NOB_INFO, "parsing: %s \n", files[i]);
		parse_file(files[i], &info);
	}
	printf("generating output, info count: %ld \n", info.count);
	generate_output(output_file, &info);
}

void generate_output(cstr *path, IntrospectableDa *data) 
{
	FILE *f = fopen(path, "w");
	// ---- Header begin ----
	fprintf(f, "#ifndef META_GEN_H_\n");
	fprintf(f, "# define META_GEN_H_\n");
	fprintf(f, "\n");

	// ---- MetaTypes ----
	fprintf(f, "typedef enum { \n");
	for (i32 i = 0; i < EncounteredTypes.count; i++) {
		fprintf(f, "\t MetaType_%s,\n", EncounteredTypes.items[i]);
	}
	fprintf(f, "\t MetaTypeCount, \n");
	fprintf(f, "} MetaTypes; \n");
	fprintf(f, "\n");

	// ---- MetaMember ----
	fprintf(f, "typedef struct { \n");
	fprintf(f, "\t MetaTypes type;\n");
	fprintf(f, "\t cstr *name;\n");
	fprintf(f, "\t size offset;\n");
	fprintf(f, "} MetaMember; \n");
	fprintf(f, "\n");

	// ---- Stuff ---

	for (i32 i = 0; i < data->count; i++) {
		Introspectable *intro = &data->items[i];
		switch (intro->type) {
			case I_Struct: {
				gen_struct(f, intro->meta_struct);
			} break;
			case I_StructTaggedUnion: {
				gen_struct(f, intro->meta_struct_tagged.meta_struct);
			} break;
			case I_Enum: {
				fprintf(f, "global cstr *%sNames[] = { \n", intro->meta_enum.name);
				for (i32 i = 0; i < intro->meta_enum.fields.count; i++) {
					fprintf(f, "\t \"%s\",\n", intro->meta_enum.fields.items[i]);
				}
				fprintf(f, "}; \n");
				fprintf(f, "\n");
			} break;
			default : {
				nob_log(NOB_ERROR, "generate not implemented for %s", IntrospectableTypesNames[intro->type]);
			} break;
		}
	}

	// ---- Header end ----
	fprintf(f, "\n");
	fprintf(f, "#endif ");
}

internal void gen_struct(FILE *f, MetaStruct s) 
{
	fprintf(f, "global MetaMember MembersOf_%s[] = { \n", s.name);
	for (i32 i = 0; i < s.fields.count; i++) {
		fprintf(f, "\t{MetaType_%s, \"%s\", offset_of(%s, %s)},\n", 
			s.fields.items[i].type, s.fields.items[i].name, s.name, s.fields.items[i].name);
	}
	fprintf(f, "}; \n");
	fprintf(f, "\n");
}

void parse_file(cstr *path, IntrospectableDa *introspectables)
{
	cstr *buf = read_file(path);
	if (buf == NULL) {
		printf("parse_file: failed to read %s. \n", path);
		return ;
	}

	Tokenizer tokenizer = {0};
	tokenizer.at = buf;
	b32 parsing = true;
	while (parsing) {
		Token token = get_token(&tokenizer);
		switch (token.type) {
			case TokenEOF: { parsing = false; } break;
			case TokenUnknown: { } break;

			case TokenIndentifier: {
				if (token_match(token, "introspect")) {
					nob_da_append(introspectables, parse_instrospectable(&tokenizer));
				}
			} break;

			default: {
//				printf("%s: %.*s \n", TokenTypeNames[token.type], token.text_size, token.text);
			} break;
		}
	}
	free(buf);
}

internal Introspectable parse_instrospectable(Tokenizer *t)
{
	if (!has_token(t, TokenOpenParan)) {
		nob_log(NOB_ERROR, "parse instrospectable: expected '(' after introspect keyword.");
	}
	IntrospectableTypes type = parse_instrospectable_params(t);
	
	Introspectable introspectable = {0};
	Token token = get_token(t);

	if (token_match(token, "typedef")) { token = get_token(t); }

	if (token_match(token, "struct")) {
		if (type == I_StructTaggedUnion) {
			introspectable.type = I_StructTaggedUnion;
			introspectable.meta_struct_tagged = parse_struct_tagged(t);
		}
		else {
			introspectable.type = I_Struct;
			introspectable.meta_struct = parse_struct(t);
		}
	}
	else if (token_match(token, "enum")) {
		introspectable.type = I_Enum;
		introspectable.meta_enum = parse_enum(t);
	} 
	else {
		nob_log(NOB_ERROR, "parse instrospectable: %.*s not implemented.", token.text_size, token.text);
	}

	return (introspectable);
}

internal IntrospectableTypes parse_instrospectable_params(Tokenizer *t)
{
	IntrospectableTypes r = I_NONE;
	while (true) {
		Token token = get_token(t);
		if (token.type == TokenCloseParan || token.type == TokenEOF) {
			break ;
		} else if (token.type == TokenIndentifier) {
			if (!token_match(token, "type")) {
				nob_log(NOB_ERROR, "parse_instrospectable_params: only 'type', param, available.");
				break ;
			}
			token = get_token(t);
			if (token.type != TokenColon) {
				nob_log(NOB_ERROR, "parse_instrospectable_params: expected ':' after indentifer like: introspectable(<indentifier>:\"<somethnig>\".");
				break ;
			}
			token = get_token(t);
			if (token.type != TokenString) {
				nob_log(NOB_ERROR, "parse_instrospectable_params: expected string after ':' like: introspectable(<indentifier>:\"<somethnig>\".");
				break ;
			} else if (token_match(token, "tagged_union")) {
				r = I_StructTaggedUnion;
			} else {
				nob_log(NOB_ERROR, "parse_instrospectable_params: %.*s not a .");
				break ;
			}
		}
	}
	return (r);
}

internal MetaEnum parse_enum(Tokenizer *t) 
{
	if (!has_token(t, TokenOpenBraces)) {
		nob_log(NOB_ERROR, "parse_enum: expected '{' after enum keyword, put enum name after '}'");
		exit(1);
	}
	MetaEnum _enum = {0};
	Token token = get_token(t);
	while (token.type != TokenCloseBraces) {
		if (token.type == TokenIndentifier) {
			nob_da_append(&_enum.fields, copy_token_text(token));
		}
		token = get_token(t);
	}
	token = get_token(t);
	_enum.name = copy_token_text(token);
	return (_enum);
}

internal MetaStruct parse_struct(Tokenizer *t) 
{
	if (!has_token(t, TokenOpenBraces)) {
		nob_log(NOB_ERROR, "parse_struct: expected '{' after struct keyword, put struct name after '}'");
		exit(1);
	}
	MetaStruct _struct = {0};
	Token token = get_token(t);
	while (token.type != TokenCloseBraces) {
		if (token_match(token, "union")) {
			nob_log(NOB_ERROR, "parse_struct: union found inside struct, ignoring.");
			parse_struct_union(t);
		} else if (token.type == TokenIndentifier) {
			nob_da_append(&_struct.fields, parse_struct_member(t, token));
		}
		token = get_token(t);
	}
	token = get_token(t);
	_struct.name = copy_token_text(token);
	return (_struct);
}

internal MetaStructTagged parse_struct_tagged(Tokenizer *t) 
{
	if (!has_token(t, TokenOpenBraces)) {
		nob_log(NOB_ERROR, "parse_struct: expected '{' after struct keyword, put struct name after '}'");
		exit(1);
	}
	MetaStructTagged st = {0};
	Token token = get_token(t);
	while (token.type != TokenCloseBraces) {
		if (token_match(token, "union")) {
			st.union_structs = parse_struct_union(t);
		} else if (token.type == TokenIndentifier) {
			nob_da_append(&st.meta_struct.fields, parse_struct_member(t, token));
		}
		token = get_token(t);
	}
	token = get_token(t);
	st.meta_struct.name = copy_token_text(token);
	return (st);
}

internal MetaMember parse_struct_member(Tokenizer *t, Token member_token)
{
	MetaMember member;
	member.type = get_type(member_token);
	Token token = get_token(t);
	while (token.type != TokenSemiColon && token.type != TokenEOF) {
		if (token.type == TokenAsterisk) {
			member.flags = FieldIsPointer;
		} 
		else if (token.type == TokenOpenBracket) {
			nob_log(NOB_INFO, "parse_struct_member: not parsing arrays yet.");
			while (token.type != TokenCloseBracket) { token = get_token(t); }
		}
		else if (token.type == TokenIndentifier) {
			member.name = copy_token_text(token);
		} 
		else {
			nob_log(NOB_ERROR, "parse_struct_member: expected * or indentifier but got: %.*s", token.text_size, token.text);
		}
		token = get_token(t);
	}
	return (member);
}

internal MetaStructDa parse_struct_union(Tokenizer *t)
{
	MetaStructDa da = {0};
	if (!has_token(t, TokenOpenBraces)) {
		nob_log(NOB_ERROR, "parse_struct_union: expected '{' after union indentifier.");
		return (da);
	}
	Token token = get_token(t);
	while (token.type != TokenCloseBraces && token.type != TokenEOF) {
		if (token_match(token, "struct")) {
			nob_da_append(&da, parse_struct_union_struct(t));
		} 
		else {
			nob_log(NOB_WARNING, "parse_struct_union: found something else than a struct inside tagged union in struct, ignoring.");
		}
		token = get_token(t);
	}
	return (da);
}

internal MetaStruct parse_struct_union_struct(Tokenizer *t)
{
	MetaStruct s = {0};
	if (!has_token(t, TokenOpenBraces)) {
		nob_log(NOB_ERROR, "parse_struct_union_struct: expected '{' after struct indentifier.");
		return (s);
	}
	Token token = get_token(t);
	while (token.type != TokenCloseBraces && token.type != TokenEOF) {
		nob_da_append(&s.fields, parse_struct_member(t, token));
		token = get_token(t);
	}
	token = get_token(t);
	s.name = copy_token_text(token);
	get_token(t); // Consume ; after name
	return (s);
}

internal b32 has_token(Tokenizer *t, TokenType token_type) 
{
	Token token = get_token(t);
	if (token.type != token_type) {
		return (false);
	}
	return (true);
}

internal b32 token_match(Token t, cstr *str) 
{
	i32 i = 0;
	while (i < t.text_size) {
		if (!str[i] || str[i] != t.text[i]) {
			return (false);
		}
		i++;
	}
	if (str[i] == '\0') return (true); // Check if we also fully traversed str
	else return (false);
}

internal Token get_token(Tokenizer *t) 
{
	while (1) {
		if (IsWhiteSpace(*t->at)) { t->at++;}
		else if (*t->at == '/' && *t->at+1 == '/') { 
			t->at += 2;
			while (*t->at && !IsEndOfLine(*t->at)) t->at++;
		}
		else if (*t->at == '/' && *t->at+1 == '*') { 
			t->at += 2;
			while (*t->at && !(*t->at == '*' && *t->at+1 == '/')) t->at++;
			if (*t->at && *t->at == '*') t->at += 2;
		}
		else {
			break;
		}
	}

	Token token = {0};
	token.text = t->at;
	token.text_size = 1;
	char c = *t->at;
	t->at++;
	switch (c) {
		case '\0': { token.type = TokenEOF;} break ;

		case '(': { token.type = TokenOpenParan; } break ;
		case ')': { token.type = TokenCloseParan; } break ;
		case ':': { token.type = TokenColon; } break ;
		case ';': { token.type = TokenSemiColon; } break ;
		case '[': { token.type = TokenOpenBracket; } break ;
		case ']': { token.type = TokenCloseBracket; } break ;
		case '{': { token.type = TokenOpenBraces; } break ;
		case '}': { token.type = TokenCloseBraces; } break ;
		case '*': { token.type = TokenAsterisk; } break ;
		case '"': {
			token.text = t->at;
			while (*t->at && *t->at != '"') {
				if (*t->at == '\\' && *t->at+1) t->at++; // Skip whatever the \ is escaping, 
				t->at++;
			}
			token.type = TokenString;
			token.text_size = t->at - token.text;
			if (*t->at == '"') t->at++;
		} break;

		default: {
			if (IsAlpha(c)) { 
				while (IsAlpha(*t->at) || IsNumber(*t->at) || *t->at == '_') {
					t->at++;
				}
				token.type = TokenIndentifier;
				token.text_size = t->at - token.text;
			}
			else if (IsNumber(c)) {
			}
			else {
				token.type = TokenUnknown;
			}
		} break;
	}
	return (token);
}

// See if we already encountered certain type, return that text, or append and return
internal cstr *get_type(Token token) 
{
	cstr *r = 0;
	for (i32 i = 0; i < EncounteredTypes.count; i++) {
		if (token_match(token, EncounteredTypes.items[i])) {
			r = EncounteredTypes.items[i];
			break ;
		}
	}
	if (!r) {
		r = copy_token_text(token);
		nob_da_append(&EncounteredTypes, r);
	}
	return (r);
}

// Read entire file to buffer and 0 terminates, allocates memory.
internal cstr *read_file(cstr *path) 
{
	cstr *buf = NULL;
	FILE *f = fopen(path, "r");
	if (f) {
		fseek(f, 0, SEEK_END);
		size f_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		buf = calloc(f_size + 1, sizeof(cstr));
		fread(buf, f_size, 1, f);
		fclose(f);
	}
	
	return (buf);
}

internal cstr *copy_token_text(Token token) 
{
	//cstr *r = calloc(token.text_size + 1, sizeof(cstr));
	cstr *r = &scratch_buf[scratch_buf_at];
	scratch_buf_at += token.text_size + 1;
	memcpy(r, token.text, token.text_size);
	return (r);
}

void print_introspectable(IntrospectableDa *da)  
{
	for (i32 i = 0; i < da->count; i++) {
		Introspectable *intro = &da->items[i];
		printf("introspectable: %s \n", IntrospectableTypesNames[intro->type]);
		switch (intro->type) {
			case I_Struct: {
				print_struct(intro->meta_struct, 1);
			} break;

			case I_StructTaggedUnion: {
				print_struct(intro->meta_struct_tagged.meta_struct, 1);
				for (i32 i = 0; i < intro->meta_struct_tagged.union_structs.count; i++) {
					print_struct(intro->meta_struct_tagged.union_structs.items[i], 2);
				}
			} break;

			case I_Enum: {
				nob_log(NOB_ERROR, "print_introspectable: printing not implemented for %s", IntrospectableTypesNames[intro->type]);
				//print_Enum(intro->meta_enum, 1);
			} break;

			default : {
				nob_log(NOB_ERROR, "print_introspectable: printing not implemented for %s", IntrospectableTypesNames[intro->type]);
			} break;
		}
	}
}

internal void print_struct(MetaStruct s, size depth)
{
	for (i32 i = 0; i < depth; i++) { printf("  "); };
	printf("Struct name: %s \n", s.name);
	for (i32 i = 0; i < s.fields.count; i++) {
		print_member(s.fields.items[i], depth + 1);
	}
}

internal void print_Enum(MetaEnum e, size depth);
internal void print_member(MetaMember m, size depth) 
{
	for (i32 i = 0; i < depth; i++) { printf("  "); };
	printf("%s %s \n", m.type, m.name);
}

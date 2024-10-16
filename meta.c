#include <stdio.h>
#include <stdlib.h>
#include "./src/modules/core.h"

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

internal cstr *read_file(cstr *path);
internal Token get_token(Tokenizer *t);
internal b32 token_match(Token t, cstr *str);
inline internal b32 IsWhiteSpace(char c);
inline internal b32 IsAlpha(char c);
inline internal b32 IsNumber(char c);
inline internal b32 IsEndOfLine(char c);

void test(cstr *path) 
{
	cstr *buf = read_file(path);
	if (buf == NULL) {
		printf("meta: failed to read %s. \n", path);
	}

	Tokenizer tokenizer;
	tokenizer.at = buf;

	b32 parsing = true;
	while (parsing) {
		Token token = get_token(&tokenizer);
		switch (token.type) {
			case TokenEOF: {
				parsing = false;
			} break;

			case TokenIndentifier: {
				if (token_match(token, "introspect")) {
					printf("token.text: %.*s\n", token.text_size, token.text);
				}
			} break;

			case TokenUnknown: { } break;

			default: {
//				printf("%s: %.*s \n", TokenTypeNames[token.type], token.text_size, token.text);
			} break;
		}
	}
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

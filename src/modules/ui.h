#ifndef XI_UI_H
# define XI_UI_H

# include "raylib.h"
# include "raymath_short.h"
# include "stdio.h"
# include "input.h"
# include "raymath.h"
# include "types.h"

typedef struct {
	Font  font;
	i32   size;
	i32   spacing;
	Color tint;
	Color tint_hover;
} FontConfig;

typedef enum {UiAlignCentralized, UiAlignLeft, UiAlignRight, UiAlignmentCount} UiAlignmentTypes_e;

typedef struct
{
	UiAlignmentTypes_e alignment;
	FontConfig font;
	b32	   draw_container_bounds;
	b32	   play_sound;
	b32	   draw_selector;
	b32	   take_key_input;
	//float	   padding_;
	f32	   padding_row;
	f32	   padding_collumn;
	f32	   padding_element; // Padding between element and its box, ex: the text in a button and the rec representing it
	f32	   padding_border; // Padding between element and border of container
	//Color	   color_;
	Color	   color_font;
	Color	   color_font_highlight;
	Color	   color_background;
	Color	   color_borders;
} UiConfig;

typedef struct
{
	V2	 pos;
	b32	 hide;
	f32	 width; // Width of the longest element added
	f32	 height; // Height of the elements NOTE  not being used, only in sliderbar
	f32	 at_x; // Position after last lement added
	f32	 at_y;
	i32	 id_count; 
	i32	 id_current;
	UiConfig config;
	// For When Title Bar is used
	// TODO  Set option to disable dragging
	b32	 mouse_dragging;
	V2	 mouse_last_pos;
} UiContainer;

void SetClickedSound(Sound *sound);
void SetSelectorTexture(Texture2D *texture);
void SetSelectorTextureTint(Color tint);

UiContainer CreateContainer(V2 pos, f32 width, UiConfig config);
UiConfig    GetDefaultUiConfig(); 
void        SetDefaultUiConfig(UiConfig config);
void        UiBegin(UiContainer *container);
void        UiEnd(UiContainer *container);
void        UiText(UiContainer *container, byte *text, b32 draw_bounds);
void        UiTextEx(UiContainer *container, UiConfig config, byte *text, b32 draw_bounds, FontConfig font);
bool        UiTextButton(UiContainer *container, byte *text);
bool        UiTextButtonEx(UiContainer *container, UiConfig config, byte *text, FontConfig font);
bool        UiSlider(UiContainer *container, f32 *value, f32 min, f32 max);
bool        UiSliderEx(UiContainer *container, UiConfig config, V2 size, f32 *value, f32 min, f32 max);
void        UiTitleBarEx(UiContainer *container, UiConfig config, byte *title, FontConfig font, Color bounds_color);

#endif

#ifdef XI_UI_IMPLEMENTATION

// NOLINTBEGIN(misc-definitions-in-headers)

static void _draw_selector(V2 offset, V2 target_bounds);
static void _take_input(UiContainer *container);
static void _play_clicked_sound();

static Texture2D *SelectorTexture = NULL; 
static Color     SelectorTint = RAYWHITE;
static Sound     *ClickedSound = NULL;
static b32       CheckMouse = false;
static UiConfig  DefaultConfig = {}; // TODO  Fill 

void SetSelectorTexture(Texture2D *texture)
{
	SelectorTexture = texture;
}

void SetSelectorTextureTint(Color tint)
{
	SelectorTint = tint;
}

void SetClickedSound(Sound *sound)
{
	ClickedSound = sound;
}

UiConfig	GetDefaultUiConfig()
{
	return (DefaultConfig);
}

void SetDefaultUiConfig(UiConfig config) 
{
	DefaultConfig = config;
}

UiContainer CreateContainer(V2 pos, f32 width, UiConfig config)
{
	return ((UiContainer) {
		.pos = pos,
		.hide = false,
		.width = width,
		.height = 0,
		.at_x = 0,
		.at_y = 0,
		.id_count = 0,
		.id_current = 0,
		.config = config,
		.mouse_dragging = false,
		.mouse_last_pos = {0, 0},
	});
}

void UiBegin(UiContainer *container) 
{
	if (container->config.draw_container_bounds && !container->hide) {
		V2	pos = container->pos;
		if (container->config.alignment == UiAlignCentralized) {
			pos.x -= container->width * 0.5f;
		}
		V2	size = {container->width, container->at_y - pos.y};
		DrawRectangleV(pos, size, container->config.color_background);
	}

	// Reseting
	container->width = 0; //  TODO Optional dynamic width
	container->at_x = container->pos.x;
	container->at_y = container->pos.y;
	container->id_count = 0;

	if (!IsMouseMoving() && WasAnyActionDown()) {
		CheckMouse = false;
	} else if (IsMouseMoving()) {
		CheckMouse = true;
	}
}

void UiEnd(UiContainer *container)
{
	if (container->config.take_key_input) _take_input(container);
}


void UiTitleBarEx(UiContainer *container, UiConfig config, byte *title, FontConfig font, Color bounds_color)
{
	V2 pos = {container->at_x, container->at_y};
	V2 text_size = MeasureTextEx(font.font, title, font.size, font.spacing);
	if (config.alignment == UiAlignCentralized) {
		pos.x -= text_size.x * 0.5f;
	}
	V2 text_pos  = {pos.x + config.padding_element, pos.y + config.padding_element};

	V2    hide_size = {text_size.x * 0.1f, text_size.y * 0.6f};
	V2    hide_pos = {text_pos.x + text_size.x + config.padding_collumn, text_pos.y + ((text_size.y - hide_size.y) * 0.5f)};
	Color hide_color = config.color_font;
	Rect  hide_rect = {hide_pos.x, hide_pos.y, hide_size.x, hide_size.y};

	V2 size = {.x = text_size.x + (config.padding_element * 2) + ((hide_pos.x + hide_size.x) - (text_pos.x + text_size.x)),
		   .y = text_size.y + (config.padding_element * 2)};
	Rect rec = {pos.x, pos.y, size.x, size.y};

	V2 mouse_pos = GetMousePosition();
	if (CheckCollisionPointRec(mouse_pos, hide_rect)) {
		hide_color = config.color_font_highlight;
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			container->hide = container->hide ? false : true;
		}
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (container->mouse_dragging) {
			container->pos = V2Add(container->pos, V2Subtract(mouse_pos, container->mouse_last_pos));
			container->mouse_last_pos = mouse_pos;
		} else if (CheckCollisionPointRec(mouse_pos, rec)) {
			container->mouse_dragging = true;
			container->mouse_last_pos = mouse_pos;
		}
	} else {
		container->mouse_dragging = false;
	}

	DrawRectangleRec(rec, bounds_color);
	DrawRectangleLinesEx(rec, 1, config.color_borders);
	DrawRectangleRec(hide_rect, hide_color);
	DrawTextEx(font.font, title, text_pos, font.size, font.spacing, config.color_font);

	container->at_y += size.y;
	container->at_y += config.padding_row; 
	if (container->width < rec.width){
		container->width  = rec.width;
	}
}

void UiText(UiContainer *container, byte *text, b32 draw_bounds) 
{
	if (container->hide == true) { return; }
	if (!IsFontReady(container->config.font.font)) {
		TraceLog(LOG_WARNING, "UiText: No default font set.");
	}
	UiTextEx(container, container->config, text, draw_bounds, container->config.font);
}

//#define Testfunc(str, ...) _Testfunc((str), (GameData){.quit = false, __VA_ARGS__})

void UiTextEx(UiContainer *container, UiConfig config, byte *text, b32 draw_bounds, FontConfig font) 
{
	if (container->hide == true) { return; }
	V2 text_size = MeasureTextEx(font.font, text, font.size, font.spacing);
	V2 pos = {container->at_x + config.padding_border, container->at_y};
	if (config.alignment == UiAlignCentralized) {
		pos.x -= text_size.x * 0.5f;
	}
	V2 text_pos = {pos.x + config.padding_element, pos.y + config.padding_element};
	V2 size = {.x = text_size.x + config.padding_element * 2,
			.y = text_size.y + config.padding_element * 2 };

	if (draw_bounds) {
		DrawRectangle(pos.x, pos.y, size.x, size.y, config.color_borders);
	}

	DrawTextEx(font.font, text, text_pos, font.size, font.spacing, config.color_font);

	container->at_y += size.y;
	container->at_y += config.padding_row; 
	if (container->width < size.x){
		container->width  = size.x;
	}
}

bool UiTextButton(UiContainer *container, byte *text)
{
	if (container->hide == true) { return(false); }
	if (!IsFontReady(container->config.font.font)) {
		TraceLog(LOG_WARNING, "UiTextButton: No default font set.");
		return (false);
	}
	return (UiTextButtonEx(container, container->config, text, container->config.font));
}

bool UiTextButtonEx(UiContainer *container, UiConfig config, byte *text, FontConfig font)
{
	if (container->hide == true) { return(false); }
	V2 text_size = MeasureTextEx(font.font, text, font.size, font.spacing);
	V2 pos = {container->at_x + config.padding_border, container->at_y};
	if (config.alignment == UiAlignCentralized) {
		pos.x -= text_size.x * 0.5f;
	}
	V2 text_pos = {pos.x + config.padding_element, pos.y + config.padding_element};
	V2 size = {.x = text_size.x + config.padding_element * 2,
			.y = text_size.y + config.padding_element * 2 };
	Rect rect = {pos.x, pos.y, size.x, size.y};

	Color color = config.color_font;
	b32 pressed = false;
	b32 mouse_inside = false;
	i32 id = container->id_count; 
	container->id_count++;

	if (CheckMouse && CheckCollisionPointRec(GetMousePosition(), rect)) {
		container->id_current = id;
		mouse_inside = true;
	}
	if (container->id_current == id) {
		DrawRectangleRec(rect, config.color_font);
		color = config.color_font_highlight;
		if (config.draw_selector) _draw_selector(pos, text_size);

		if ((mouse_inside && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) || IsActionPressed(ACTION_1)) {
			pressed = true;
			if (config.play_sound) _play_clicked_sound();
		}
	} else {
		DrawRectangleRec(rect, config.color_font_highlight);
	}

	DrawTextEx(font.font, text, text_pos, font.size, font.spacing, color);

	container->at_y += size.y;
	container->at_y += config.padding_row;
	if (container->width < size.x){
		container->width  = size.x;
	}
	return (pressed);
}

bool UiSlider(UiContainer *container, f32 *value, f32 min, f32 max)
{
	if (container->hide == true) { return(false); }
	V2 size = {container->width * 0.8f, container->height};
	return (UiSliderEx(container, container->config, size, value, min, max));
}

bool UiSliderEx(UiContainer *container, UiConfig config, V2 size, f32 *value, f32 min, f32 max)
{
	if (container->hide == true) { return(false); }
	b32  pressed = false;
	b32  mouse_inside = false;
	V2   offset = {container->at_x, container->at_y};
	Rect rect = {offset.x, offset.y, size.x, size.y};
	i32  id = container->id_count; 
	f32  step = 0.1f;
	container->id_count++;

	if (config.alignment == UiAlignCentralized) {
		offset.x -= size.x * 0.5f;
		rect.x = offset.x;
	}

	if (CheckMouse && CheckCollisionPointRec(GetMousePosition(), rect)) {
		container->id_current = id;
		mouse_inside = true;
	}
	if (container->id_current == id) {
		if (config.draw_selector) _draw_selector(offset, size);
		
		if (mouse_inside) {
			f32 wheel = GetMouseWheelMove();
			if (wheel == 1.0f || wheel == -1.0f) {
				*value += step * wheel;	
				pressed = true;
			}
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
				V2	pos = Vector2Subtract(GetMousePosition(), (V2){rect.x, rect.y});
				*value = Remap(pos.x, 0, rect.width, 0, 1);
				pressed = true;
			}
		}

		if (IsActionPressed(RIGHT)) {
			*value += step;	
			pressed = true;
		}
		if (IsActionPressed(LEFT)) {
			*value -= step;	
			pressed = true;
		}
	}

	if (*value > max) *value = max;
	if (*value < min) *value = min;

	Rect rect2 = {rect.x, rect.y, rect.width * (*value), rect.height};

	DrawRectanglePro(rect, (V2){0,0}, 0, GREEN);
	DrawRectanglePro(rect2, (V2){0,0}, 0, BLUE);

	container->at_y += size.y;
	container->at_y += config.padding_row;
	if (container->width < size.x){
		container->width  = size.x;
	}
	if (pressed && config.play_sound) _play_clicked_sound();
	return (pressed);
}

static void _draw_selector(V2 offset, V2 target_bounds)
{
	if (SelectorTexture == NULL) { 
		TraceLog(LOG_WARNING, "Ui: draw_selector requested but no texture set.");
		return ;
	}
	DrawTexturePro(
		*SelectorTexture,
		(Rect){0, 0, (float)SelectorTexture->width, (float)SelectorTexture->height},
		(Rect){offset.x - (SelectorTexture->width * 0.50f) - 5, offset.y + (target_bounds.y * 0.5f) - (SelectorTexture->height * 0.25f), SelectorTexture->width * 0.5f, SelectorTexture->height * 0.5f},
		(V2){0,0},
		0,
	SelectorTint);
}

static void _play_clicked_sound()
{
	if (ClickedSound == NULL) {
		TraceLog(LOG_WARNING, "Ui: play_sound requested but no sound set.");
		return ;
	}
	PlaySound(*ClickedSound);
}

static void _take_input(UiContainer *container)
{
	i32 *current = &container->id_current;
	i32 *count = &container->id_count;
	if (IsActionPressed(UP)) {
		*current -= 1;
		if (*current < 0) {
			*current = *count - 1;
		}
	}
	if (IsActionPressed(DOWN)) {
		*current += 1;
		if (*current >= *count) {
			*current = 0;
		}
	}
}

// void _Testfunc(char *str, GameData data) 
// {
// 	printf("str: %s, data- quit: %d, music_vol: %f\n", str, data.quit, data.music_vol);
// }

// #define Testfunc(str, ...) _Testfunc((str), (GameData){.quit = false, __VA_ARGS__})
// NOLINTEND(misc-definitions-in-headers)
#endif

#ifndef XI_UI_H
# define XI_UI_H

# include "raylib.h"
# include "stdio.h"
# include "input.h"
# include "raymath.h"
# include "types.h"

typedef struct {
	Font	font;
	int	size;
	int	spacing;
	Color	tint;
	Color	tint_hover;
} FontConfig;

typedef struct UiPanel
{
	V2	pos; // NOTE  Remember initialize this
	float	width; // Width of the longest element added
	float	height; // Height of the elements
	float	at_x; // Position after last lement added
	float	at_y;
	int	id_count; 
	int	id_current; // NOTE  Remember to 0 initialize this
	bool	centralized; // if true centralizes elements on given pos and at panel_end changes pos.x to give a proper bounding box of the panel
} UiPanel;


void	set_clicked_sound(Sound *sound);
void	set_selector_texture(Texture2D *texture);
void	set_selector_texture_tint(Color tint);

void	panel_begin(UiPanel *panel);
void	panel_end(UiPanel *panel);
void	panel_take_key_input(UiPanel *panel);
void	panel_text(UiPanel *panel, char *text, FontConfig font);
bool	panel_text_button(UiPanel *panel, char *text, FontConfig config);
bool	panel_slider(UiPanel *panel, float *value, float min, float max);
bool	panel_slider_sized(UiPanel *panel, V2 size, float *value, float min, float max);

#endif

#ifdef XI_UI_IMPLEMENTATION

// NOLINTBEGIN(misc-definitions-in-headers)
//
void	DrawSelector(V2 offset, V2 target_bounds);

// IDK MAN AOETNHOANTEH
static Texture2D	*SelectorTexture = NULL; 
static Color		SelectorTint = RAYWHITE;
static Sound		*ClickedSound = NULL;
static bool		CheckMouse = false;

void	set_selector_texture(Texture2D *texture)
{
	SelectorTexture = texture;
}

void	set_selector_texture_tint(Color tint)
{
	SelectorTint = tint;
}

void	set_clicked_sound(Sound *sound)
{
	ClickedSound = sound;
}

void	panel_begin(UiPanel *panel) 
{
	panel->width = 0;
	panel->at_x = panel->pos.x;
	panel->at_y = panel->pos.y;
	panel->id_count = 0;

	if (!IsMouseMoving() && WasAnyActionDown()) {
		CheckMouse = false;
	} else if (IsMouseMoving()) {
		CheckMouse = true;
	}
}

void	panel_end(UiPanel *panel)
{
	panel_take_key_input(panel);
	if (panel->centralized) {
		panel->pos.x -= panel->width * 0.5f;
	}
}

void	panel_take_key_input(UiPanel *panel)
{
	int	*current = &panel->id_current;
	int	*count = &panel->id_count;
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

// TODO  Pass different padding 
void	panel_text(UiPanel *panel, char *text, FontConfig font) 
{
	V2	text_size = MeasureTextEx(font.font, text, font.size, font.spacing);
	V2	offset = {panel->at_x, panel->at_y};
	if (panel->centralized) {
		offset.x -= text_size.x * 0.5f;
	}

	// TODO  Option for drawing rectangle behind text?
	//DrawRectangle(offset.x, offset.y, text_size.x, text_size.y, font.tint_hover);
	DrawTextEx(font.font, text, offset, font.size, font.spacing, font.tint);

	panel->at_y += text_size.y;
	panel->at_y += 15; // Add padding // NOTE  Hard coded value
	if (panel->width < text_size.x){
		panel->width  = text_size.x;
	}
}

bool	panel_text_button(UiPanel *panel, char *text, FontConfig config)
{
	bool	pressed = false;
	bool	mouse_inside = false;
	V2	text_size = MeasureTextEx(config.font, text, config.size, config.spacing);
	V2	offset = {panel->at_x, panel->at_y};
	Rect	rect = {offset.x, offset.y, text_size.x, text_size.y};
	Color	color = config.tint;
	int	id = panel->id_count; 
	panel->id_count++;

	if (panel->centralized) {
		offset.x -= text_size.x * 0.5f;
		rect.x = offset.x;
	}

	if (CheckMouse && CheckCollisionPointRec(GetMousePosition(), rect)) {
		panel->id_current = id;
		mouse_inside = true;
	}
	if (panel->id_current == id) {
		DrawRectangleV((V2){rect.x - 2, rect.y}, (V2){rect.width + 2, rect.height + 1}, config.tint);
		color = config.tint_hover;
//		draw_selector_cursor((V2){panel->at_x, panel->at_y + text_size.y * 0.5f});
		//DrawTexture(SelectorTexture, offset.x - SelectorTexture.width - 5, offset.y + (text_size.y * 0.5f) - (SelectorTexture.height * 0.5f), SelectorTint);
		// TODO  Refactor into function that draws texture scaled
		DrawSelector(offset, text_size);

		if ((mouse_inside && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) || IsActionPressed(ACTION_1)) {
			pressed = true;
			if (ClickedSound) {
				PlaySound(*ClickedSound);
			}
		}
	} else {
		DrawRectangleV((V2){rect.x - 2, rect.y}, (V2){rect.width + 2, rect.height}, config.tint_hover);
	}

	DrawTextEx(config.font, text, offset, config.size, config.spacing, color);

	panel->at_y += text_size.y;
	panel->at_y += 10; // Padding // NOTE  Hard coded
	if (panel->width < text_size.x){
		panel->width  = text_size.x;
	}
	return (pressed);
}

bool	panel_slider(UiPanel *panel, float *value, float min, float max)
{
	V2	size = {panel->width * 0.8f, panel->height};
	return (panel_slider_sized(panel, size, value, min, max));
}

bool	panel_slider_sized(UiPanel *panel, V2 size, float *value, float min, float max)
{

	bool	pressed = false;
	bool	mouse_inside = false;
	V2	offset = {panel->at_x, panel->at_y};
	Rect	rect = {offset.x, offset.y, size.x, size.y};
	int	id = panel->id_count; 
	float	step = 0.1f;
	panel->id_count++;

	if (panel->centralized) {
		offset.x -= size.x * 0.5f;
		rect.x = offset.x;
	}

	if (CheckMouse && CheckCollisionPointRec(GetMousePosition(), rect)) {
		panel->id_current = id;
		mouse_inside = true;
	}
	if (panel->id_current == id) {
		DrawSelector(offset, size);
		
		if (mouse_inside) {
			float wheel = GetMouseWheelMove();
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
			if (ClickedSound) PlaySound(*ClickedSound);
			pressed = true;
		}
		if (IsActionPressed(LEFT)) {
			*value -= step;	
			if (ClickedSound) PlaySound(*ClickedSound);
			pressed = true;
		}
	}

	if (*value > max) *value = max;
	if (*value < min) *value = min;

	Rect	rect2 = {rect.x, rect.y, rect.width * (*value), rect.height};

	DrawRectanglePro(rect, (V2){0,0}, 0, GREEN);
	DrawRectanglePro(rect2, (V2){0,0}, 0, BLUE);


	panel->at_y += size.y;
	panel->at_y += 10; // Padding // NOTE  Hard coded
	if (panel->width < size.x){
		panel->width  = size.x;
	}
	if (pressed) PlaySound(*ClickedSound);
	return (pressed);
}

void	DrawSelector(V2 offset, V2 target_bounds)
{
	if (SelectorTexture) { 
		DrawTexturePro(
			*SelectorTexture,
			(Rect){0, 0, (float)SelectorTexture->width, (float)SelectorTexture->height},
			(Rect){offset.x - (SelectorTexture->width * 0.50f) - 5, offset.y + (target_bounds.y * 0.5f) - (SelectorTexture->height * 0.25f), SelectorTexture->width * 0.5f, SelectorTexture->height * 0.5f},
			(V2){0,0},
			0,
		SelectorTint);
	}
}

// NOLINTEND(misc-definitions-in-headers)
#endif

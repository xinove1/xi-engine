#ifndef MUI_H_
# include "microui.h"
# include "core.h"
# include "raylib.h"
# include "sprite.h"

#define ColorFromMu(c) ((Color){c.r, c.g, c.b, c.a})
#define ColorToMu(c) ((mu_Color){c.r, c.g, c.b, c.a})
#define RectFromMu(r) ((Rect){r.x, r.y, r.w, r.h})
#define RectFromMuFixed(r) ((Rect){r.x +1, r.y +1, r.w -1, r.h -1})
#define RectFromMuFixed2(r) ((Rect){r.x +1, r.y +1, r.w, r.h})
#define MuRec(x, y, width, height) (mu_Rect) {x, y, width, height}
#define MuRecV2(pos, size) (mu_Rect) {.x = pos.x, .y = pos.y, .w = size.x, .h = size.y}
#define MuRectExpand(r, amount) ((mu_Rect) {r.x - amount, r.y - amount, r.w + amount * 2, r.h + amount * 2});
#define V2FromMu(v) ((V2){v.x, v.y})
#define V2ToMu(v) ((mu_Vec2){v.x, v.y})

void MUiInit(mu_Context *ctx, Font *font, V2 canvas_size);
void MUiSetSpacing(int spacing);
void MUiPoolInput(mu_Context *ctx);
void MUiRender(mu_Context *ctx);
b32 MUiLoadStyle(mu_Context *ctx, cstr *file);
b32 MUiSaveStyle(mu_Context *ctx, cstr *file);
b32 MUiLoadStyleColors(mu_Context *ctx, cstr *file); // For adding stuff to mu_Style and preserving previous colors
b32 MUiSaveStyleColors(mu_Context *ctx, cstr *file); 

b32 MUiIsMouseInsideContainer(mu_Context *ctx);
i32 u8_slider(mu_Context *ctx, u8 *value, i32 low, i32 high);
int MUiToggleButtonEx(mu_Context *ctx, int *state, int opt);
int MUiTextureButton(mu_Context *ctx, Sprite *texture, int opt) ;

#endif

#ifdef MUI_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)
#define GetMuFont(f) ((f == NULL) ? GetFontDefault() : *(Font *)f)

typedef struct {
	KeyboardKey ray;
	i32 mu;
} _MUiKeys;

typedef struct {
  MouseButton ray;
  i32 mu;
} _MUiMouse;

global _MUiMouse _MouseMatches[] = {
	{MOUSE_BUTTON_LEFT, MU_MOUSE_LEFT},
	{MOUSE_BUTTON_RIGHT, MU_MOUSE_RIGHT},
	{MOUSE_BUTTON_MIDDLE, MU_MOUSE_MIDDLE},
};

global _MUiKeys _KeyMatches[] = {
	{KEY_LEFT_SHIFT, MU_KEY_SHIFT},    {KEY_RIGHT_SHIFT, MU_KEY_SHIFT},
	{KEY_LEFT_CONTROL, MU_KEY_CTRL},   {KEY_RIGHT_CONTROL, MU_KEY_CTRL},
	{KEY_LEFT_ALT, MU_KEY_ALT},        {KEY_RIGHT_ALT, MU_KEY_ALT},
	{KEY_ENTER, MU_KEY_RETURN},        {KEY_KP_ENTER, MU_KEY_RETURN},
	{KEY_BACKSPACE, MU_KEY_BACKSPACE}
};

global int TextSpacing = 1;

void MUiSetSpacing(int spacing)
{
	Assert(spacing > 0);
	TextSpacing = spacing;
}

int _get_text_width(mu_Font font, const char *str, int len)
{
	Font rfont = GetMuFont(font);
	Vector2 size = {0};
	if (len == -1) {
		size = MeasureTextEx(rfont, str, rfont.baseSize, TextSpacing);
	} else {
		//TraceLog(LOG_INFO, "_get_text_width: len != -1");
		char buf[200] = {0};
		memcpy(buf, str, len);
		buf[len] = 0;
		size = MeasureTextEx(rfont, buf, rfont.baseSize, TextSpacing);
	}
	return (size.x);
}

int _get_text_height(mu_Font font)
{
	Font rfont = GetMuFont(font);
	return (rfont.baseSize);
}

void MUiInit(mu_Context *ctx, Font *font, V2 canvas_size)
{
	mu_init(ctx);
	ctx->style->font = (mu_Font) font;
	ctx->text_width = _get_text_width;
	ctx->text_height = _get_text_height;
	ctx->get_frame_time = GetFrameTime;
	ctx->tooltip_time = 0.4f;
	ctx->style->spacing = TextSpacing; // TODO what
	ctx->window_size = V2ToMu(canvas_size);
	if (canvas_size.x <= 0 && canvas_size.y <= 0) {
		TraceLog(LOG_WARNING, "MUiInit: canvas size is invalid, tooltip behavior will not work.");
	}
}

void MUiPoolInput(mu_Context *ctx)
{
	V2 mouse = GetMousePosition();
	mu_input_mousemove(ctx, mouse.x, mouse.y);

	V2 scroll = GetMouseWheelMoveV();
	mu_input_scroll(ctx, scroll.x * -30, scroll.y * -30); // Negative because by default its inverted, and 30 to increase the speed
	
	// Keys
	i32 _size = count_of(_KeyMatches);
	for (i32 i = 0; i < _size; i++) {
		_MUiKeys k = _KeyMatches[i];
		if (IsKeyPressed(k.ray)) mu_input_keydown(ctx, k.mu);
		if (IsKeyReleased(k.ray)) mu_input_keyup(ctx, k.mu);
	}

	// Mouse Clickes
	_size = count_of(_MouseMatches);
	for (i32 i = 0; i < _size; i++) {
		_MUiMouse k = _MouseMatches[i];
		if (IsMouseButtonPressed(k.ray)) mu_input_mousedown(ctx, mouse.x, mouse.y, k.mu);
		if (IsMouseButtonReleased(k.ray)) mu_input_mouseup(ctx, mouse.x, mouse.y, k.mu);
	}

	// Text Input
	cstr buf[100] = {0};
	for (i32 i = 0; i < 100; i++) {
		char c = GetCharPressed();
		buf[i] = c;
		if (c == '\0') break ;
	}
	mu_input_text(ctx, buf);
}

void MUiRender(mu_Context *ctx)
{
	Assert(ctx && "Please pass context in");
	BeginScissorMode(0, 0, GetScreenWidth(), GetScreenHeight());
	mu_Command *cmd = NULL;
	while (mu_next_command(ctx, &cmd)) {
		switch (cmd->type) {
			case MU_COMMAND_TEXT: {
				cmd->text.pos.x -= 1;
				cmd->text.pos.y -= 1;
				Font font = GetMuFont(cmd->text.font);
				DrawTextEx(font, cmd->text.str, V2FromMu(cmd->text.pos), ctx->text_height(&font), TextSpacing, ColorFromMu(cmd->text.color));
			} break ;
			case MU_COMMAND_RECT: {
				cmd->rect.rect.x -= 1;
				cmd->rect.rect.y -= 1;
				DrawRectangleRec(RectFromMuFixed(cmd->rect.rect), ColorFromMu(cmd->rect.color));
			} break;
			case MU_COMMAND_RECT_BORDER: {
				cmd->rect.rect.x -= 1;
				cmd->rect.rect.y -= 1;
				DrawRectangleLinesEx(RectFromMuFixed(cmd->rect.rect), 1, ColorFromMu(cmd->rect.color));
			} break;
			case MU_COMMAND_TEXTURE: {
				cmd->texture.rect.x -= 1;
				cmd->texture.rect.y -= 1;
				Sprite sprite = *(Sprite *)cmd->texture.texture;
				sprite.pos = Vec2(cmd->texture.rect.x, cmd->texture.rect.y);
				sprite.size = Vec2(cmd->texture.rect.w, cmd->texture.rect.h);
				sprite.tint = ColorFromMu(cmd->texture.color);
				DrawSprite(sprite);
			} break;
			case MU_COMMAND_ICON: { 
				cmd->icon.rect.x -= 1;
				cmd->icon.rect.y -= 1;
				Color color = ColorFromMu(cmd->icon.color);
				cstr icon[2] = "!";
				switch (cmd->icon.id) {
					case MU_ICON_CLOSE: icon[0] = 'x'; break;
					case MU_ICON_CHECK: icon[0] = '*'; break;
					case MU_ICON_COLLAPSED: icon[0] = '+' ; break;
					case MU_ICON_EXPANDED: icon[0] = '-'; break;
					default: Assert(0 && "unreachable");
				}
				DrawText(icon, cmd->icon.rect.x + 1, cmd->icon.rect.y + 1, cmd->icon.rect.h, color);
			} break;
			case MU_COMMAND_CLIP: { 
				cmd->clip.rect.x -= 1;
				cmd->clip.rect.y -= 1;
				EndScissorMode();
				BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w, cmd->clip.rect.h);
			} break;
		}
	}
	EndScissorMode();
}

b32 MUiIsMouseInsideContainer(mu_Context *ctx) 
{
	V2 mouse_pos = GetMousePosition();
	for (int i = 0; i < MU_CONTAINERPOOL_SIZE; i++) {
		mu_Container container = ctx->containers[i];
		if (!container.open) continue;
		Rect rec = RectFromMu(container.rect);
		if (CheckCollisionPointRec(mouse_pos, rec)) return (true);
	}
	return (false);
}

b32 MUiSaveStyle(mu_Context *ctx, cstr *file) 
{
	// NOTE  Save only data after mu_Font(void *) so we don't need to care about the system pointer size when reading the data later. We can't save the font so we don't lose anything.
	b32 r = SaveFileData(file, (&ctx->style->size), sizeof(mu_Style) - ((void *)&ctx->style->size - (void *)ctx->style));
	return (r);
}

b32 MUiSaveStyleColors(mu_Context *ctx, cstr *file) 
{
	b32 r = SaveFileData(file, &ctx->style->colors, sizeof(ctx->style->colors));
	return (r);
}

b32 MUiLoadStyle(mu_Context *ctx, cstr *file)
{
	b32 r = false;
	i32 data_read = 0;
	i32 data_expected = sizeof(mu_Style) - ((void *)&ctx->style->size - (void *)ctx->style);
	unsigned char *data = LoadFileData(file, &data_read);
	if (!data) return (false) ;
	if (data_read != data_expected) {
		TraceLog(LOG_WARNING, "MUiLoadStyle: amount of data read from file does not match size of mu_Style, not copying into context. data read %d, data expected: %d", data_read, data_expected);
	} else {
		memcpy(&ctx->style->size, data,  data_expected);
		r = true;
	}
	UnloadFileData(data);
	return (r);
}

b32 MUiLoadStyleColors(mu_Context *ctx, cstr *file) 
{
	b32 r = false;
	i32 data_read = 0;
	unsigned char *data = LoadFileData(file, &data_read);
	if (!data) return (false) ;
	if (data_read != sizeof(ctx->style->colors)) {
		TraceLog(LOG_WARNING, "MUiLoadStyleColors: amount of data read from file does not match size of this context colors array, not copying into context. data read %d, data expected: %d", data_read, sizeof(ctx->style->colors));
	} else {
		memcpy(ctx->style->colors, data, sizeof(ctx->style->colors));
		r = true;
	}
	UnloadFileData(data);
	return (r);
}

i32 u8_slider(mu_Context *ctx, u8 *value, i32 low, i32 high) 
{
	f32 tmp;
	mu_push_id(ctx, &value, sizeof(value));
	tmp = *value;
	i32 res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
	*value = tmp;
	mu_pop_id(ctx);
	return res;
}

int MUiToggleButtonEx(mu_Context *ctx, int *state, int opt) 
{
	int res = 0;
	mu_Id id = mu_get_id(ctx, &state, sizeof(state));
	mu_Rect r = mu_layout_next(ctx);
	mu_Rect box = MuRectExpand(r, -2);
	mu_update_control(ctx, id, r, opt);
	/* handle click */
	if (ctx->mouse_pressed == MU_MOUSE_LEFT && ctx->focus == id) {
		res |= MU_RES_CHANGE;
		*state = *state ? false : true;
	}
	/* draw */
	mu_draw_control_frame(ctx, id, r, MU_COLOR_BUTTON, opt);
	if (*state) {
	//	mu_draw_icon(ctx, MU_ICON_CHECK, r, ctx->style->colors[MU_COLOR_TEXT]);
		mu_draw_control_frame(ctx, id, box, MU_COLOR_BASE, opt);
	}
	return res;
}

int MUiTextureButton(mu_Context *ctx, Sprite *texture, int opt) 
{
	int res = 0;
	mu_Id id = mu_get_id(ctx, texture, sizeof(*texture));
	mu_Rect r = mu_layout_next(ctx);
	mu_update_control(ctx, id, r, opt);
	/* handle click */
	if (ctx->mouse_pressed == MU_MOUSE_LEFT && ctx->focus == id) {
		res |= MU_RES_SUBMIT;
	}
	/* draw */
	mu_draw_control_frame(ctx, id, r, MU_COLOR_BUTTON, opt);
	if (texture) {
		mu_Color color = ColorToMu(texture->tint);
		if (ctx->focus == id) {
			color = ctx->style->colors[MU_COLOR_BUTTONHOVER];
		} else if (ctx->hover == id) {
			color = ctx->style->colors[MU_COLOR_BUTTONFOCUS];
		}
		mu_Rect texture_rect = r;
		if (texture->size.x <= 0 || texture->size.y <= 0) {
			TraceLog(LOG_WARNING, "MUiTextureButton: texture size is %d, %d", texture->size.x, texture->size.y);
		}
		if (~opt & MU_OPT_STRETCH_TEXTURE) {
			texture_rect.w = texture->size.x;
			texture_rect.h = texture->size.y;
		}
		if (opt & MU_OPT_ALIGNRIGHT && r.w > texture->size.x) {
			texture_rect.x = r.w - texture->size.x;
		}
		else if (opt & MU_OPT_ALIGNCENTER) {
			if (r.w > texture->size.x) {
				i32 diff = r.w - texture->size.x;
				texture_rect.x += diff * 0.5f;
			}
			if (r.h > texture->size.y) {
				i32 diff = r.h - texture->size.y;
				texture_rect.y += diff * 0.5f;
			}
		}
		mu_draw_texture(ctx, texture, texture_rect, color);
	} else {
		TraceLog(LOG_WARNING, "MUiTextureButton: null pointer passed as texture");
	}

	return res;
}

// NOLINTEND(misc-definitions-in-headers)
#endif

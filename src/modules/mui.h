#ifndef MUI_H_
# include "microui.h"
# include "core.h"
# include "raylib.h"

void MUiInit(mu_Context *ctx, Font *font);
void MUiSetSpacing(int spacing);
void MUiPoolInput(mu_Context *ctx);
void MUiRender(mu_Context *ctx);

#endif

#ifdef MUI_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

#define ColorFromMu(c) ((Color){c.r, c.g, c.b, c.a})
#define RectFromMu(r) ((Rect){r.x, r.y, r.w, r.h})
#define V2FromMu(v) ((V2){v.x, v.y})
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
	(void) len;
	Font rfont = GetMuFont(font);
	Vector2 size = MeasureTextEx(rfont, str, rfont.baseSize, TextSpacing);
	return (size.x);
}

int _get_text_height(mu_Font font)
{
	Font rfont = GetMuFont(font);
	return (rfont.baseSize);
}

void MUiInit(mu_Context *ctx, Font *font)
{
	mu_init(ctx);
	ctx->style->font = (mu_Font) font;
	ctx->text_width = _get_text_width;
	ctx->text_height = _get_text_height;
	ctx->style->spacing = TextSpacing;
}

void MUiPoolInput(mu_Context *ctx)
{
	V2 mouse = GetMousePosition();
	mu_input_mousemove(ctx, mouse.x, mouse.y);

	V2 scroll = GetMouseWheelMoveV();
	mu_input_scroll(ctx, scroll.x * -30, scroll.y * -30); // TODO  test without this multiplication
	
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
	//printf("hi from render \n");
	mu_Command *cmd = NULL;
	while (mu_next_command(ctx, &cmd)) {
		switch (cmd->type) {
			case MU_COMMAND_TEXT: {
				//r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
				Font font = GetMuFont(cmd->text.font);
				DrawTextEx(font, cmd->text.str, V2FromMu(cmd->text.pos), ctx->text_height(&font), TextSpacing, ColorFromMu(cmd->text.color));
			} break ;
			case MU_COMMAND_RECT: {
				///r_draw_rect(cmd->rect.rect, cmd->rect.color);
				DrawRectangleRec(RectFromMu(cmd->rect.rect), ColorFromMu(cmd->rect.color));
			} break;
			case MU_COMMAND_ICON: { 
				//r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); 
				Color color = ColorFromMu(cmd->icon.color);
				cstr icon[2] = "!";
				switch (cmd->icon.id) {
					case MU_ICON_CLOSE: icon[0] = 'x'; break;
					case MU_ICON_CHECK: icon[0] = '*'; break;
					case MU_ICON_COLLAPSED: icon[0] = '+' ; break;
					case MU_ICON_EXPANDED: icon[0] = '-'; break;
					default: Assert(0 && "unreachable");
				}
				DrawText(icon, cmd->icon.rect.x, cmd->icon.rect.y, cmd->icon.rect.h, color);
			} break;
			case MU_COMMAND_CLIP: { 
				EndScissorMode();
				BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w, cmd->clip.rect.h);
				//r_set_clip_rect(cmd->clip.rect); 
			} break;
		}
	}
	EndScissorMode();
}

// NOLINTEND(misc-definitions-in-headers)
#endif

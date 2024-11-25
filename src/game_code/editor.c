#include "game.h"
#include "./microui_exemple.c"

global GameEditor *Ed = NULL;

internal void edit_entity(Entity *e);
internal void is_entity_under_mouse(Entity *e);

void init_editor(GameData *data)
{
	TraceLog(LOG_INFO, "Initting Editor");
	Data = data;
	Ed = &data->editor;

	// TODO  Change game and editor to use same mu context
	Ed->mu = calloc(1, sizeof(mu_Context));
	MUiInit(Ed->mu, &Data->font, Data->canvas_size);
	MUiLoadStyle(Ed->mu, "assets/ui_style");
	Data->ui.mu->style = Ed->mu->style;
}

void pos_reload_editor(GameData *data) 
{
	Ed = &data->editor;
}

void update_editor()
{
	V2 mouse_pos = GetMousePosition();

	if (IsKeyPressed(KEY_U)) {
		Ed->debug_window = Ed->debug_window ? false : true;
	}

	// ----- Ui -----
	MUiPoolInput(Ed->mu);
	mu_begin(Ed->mu); {
		mu_Context *ctx = Ed->mu;
		if (Ed->debug_window) {
			if (mu_begin_window(ctx, "Debug window", mu_rect(Data->canvas_size.x * 0.1f, Data->canvas_size.y * 0.1f, 150, 200))) {
				mu_layout_row(ctx, 2, (int[]){0, -1}, 0);
				mu_label(ctx, "Tower infinity health");
				MUiToggleButtonEx(ctx, &Ed->no_lose, 0);
				mu_label(ctx, "Debug select");
				if (MUiToggleButtonEx(ctx, &Ed->debug_select, 0) && Ed->debug_select == false) {
					Ed->hovered = NULL;
					Ed->selected = NULL;
				}
				mu_layout_row(ctx, 2, (int[]){0, 0}, 0);
				mu_tooltip(ctx, "Save Style to assets/ui_style i ea osenutha oesnuthaoesntueh aontsuhe");
				if (mu_button(ctx, "Save Style")) {
					MUiSaveStyle(ctx, "assets/ui_style");
				}
				if (mu_button(ctx, "Save Color")) {
					MUiSaveStyleColors(ctx, "assets/ui_style_colors");
				}
				mu_tooltip(ctx, "Load Style from file");
				if (mu_button(ctx, "Load Style")) {
					MUiLoadStyle(ctx, "assets/ui_style");
				}
				mu_tooltip(ctx, "Load Color from file");
				if (mu_button(ctx, "Load Color")) {
					MUiLoadStyleColors(ctx, "assets/ui_style_colors");
				}
				mu_label(ctx, "teste sizla");
				mu_end_window(ctx);
			}
			style_window(Ed->mu);
			Data->ui.mu->style = Ed->mu->style;
		}
		//process_frame(Ed->mu);
	} mu_end(Ed->mu);

	if (MUiIsMouseInsideContainer(Ed->mu)) return ;

	// NOTE  Nothing bellow this runs if mouse is inside a ui window

	// ----- Editor ----- 
	if (Ed->debug_select) {
		Ed->hovered = NULL;

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if (Ed->hovered) {
				Ed->selected = Ed->hovered;
				Ed->hovered = NULL;

				// If clicking on same entity that window was closed, open it back up
			} else Ed->selected = NULL;
		}
	} 

}

void draw_editor()
{
	Color selected_highlight = BLACK;
	Color hover_highlight = PURPLE;
	if (Ed->selected) {
		DrawRectangleLinesEx(RecV2(Ed->selected->pos, Ed->selected->render.size), 1, selected_highlight);
	}
	if (Ed->hovered) {
		DrawRectangleLinesEx(RecV2(Ed->hovered->pos, Ed->hovered->render.size), 1, hover_highlight);
	}

	MUiRender(Ed->mu);
}

internal void edit_entity(Entity *e) 
{
	mu_Context *ctx = Ed->mu;

	//mu_label(ctx, EntityTypeNames[e->type]);
	i32 body_width = mu_get_current_container(ctx)->body.w;
	i32 width = body_width * 0.30;
	i32 label_w = 50;

	mu_layout_row(ctx, 2, (int[]) {label_w, -1}, 0);
	if (mu_header(ctx, "Effects")) {
		mu_layout_row(ctx, 2, (int[]) {label_w, -1}, 0);
		mu_label(ctx, "Duration");
		mu_slider(ctx, &Ed->effect_duration, 0.01, 0.99);
		i32 width_ = body_width * 0.15;
		mu_layout_row(ctx, 6, (int[]) {label_w, width_, width_, width_, width_, -1}, 0);
		mu_label(ctx, "Color");
		u8_slider(ctx, &Ed->effect_color.r, 0, 255);
		u8_slider(ctx, &Ed->effect_color.g, 0, 255);
		u8_slider(ctx, &Ed->effect_color.b, 0, 255);
		u8_slider(ctx, &Ed->effect_color.a, 0, 255);
		mu_draw_rect(ctx, mu_layout_next(ctx), ColorToMu(Ed->effect_color));
	}
	if (mu_button(ctx, "apply")) {
		apply_flash_effect(e, Ed->effect_color, Ed->effect_duration);
		//apply_shake_effect(entity, 0.25f);
	}

	mu_layout_row(ctx, 3, (int[]) {label_w, width, -1}, 0);
	mu_label(ctx, "Position");
	mu_number_ex(ctx, &e->pos.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->pos.y, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_label(ctx, "Size");
	mu_number_ex(ctx, &e->render.size.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->render.size.y, 3, "%.1f", MU_OPT_ALIGNCENTER);

	width = body_width * 0.14;
	mu_layout_row(ctx, 6, (int[]) {label_w, width, width, width, width, -1}, 0);
	mu_label(ctx, "Color");
	u8_slider(ctx, &e->render.tint.r, 0, 255);
	u8_slider(ctx, &e->render.tint.g, 0, 255);
	u8_slider(ctx, &e->render.tint.b, 0, 255);
	u8_slider(ctx, &e->render.tint.a, 0, 255);
	mu_draw_rect(ctx, mu_layout_next(ctx), ColorToMu(e->render.tint));
}

internal void is_entity_under_mouse(Entity *e) 
{
	V2 mouse_pos = GetMousePosition();
	Rect e_rec = RecV2(e->pos, e->render.size);
	if (CheckCollisionPointRec(mouse_pos, e_rec)) {
		Ed->hovered = e;
		return ;
	}
}

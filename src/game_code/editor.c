#include "game.h"
#include "./microui_exemple.c"

global GameEditor *Ed = NULL;

internal void edit_entity(Entity *e);
internal void get_entity_under_mouse();

void init_editor(GameData *data)
{
	TraceLog(LOG_INFO, "Initting Editor");
	Data = data;
	Ed = &data->editor;

	Ed->mu = calloc(1, sizeof(mu_Context));
	MUiInit(Ed->mu, NULL);
}

void pos_reload_editor(GameData *data) 
{
	Ed = &data->editor;
}

void update_editor()
{
	V2 mouse_pos = GetMousePosition();

	if (IsKeyPressed(KEY_D)) {
		Ed->debug_window = Ed->debug_window ? false : true;
	}
	if (Ed->no_lose) {
		Data->level->cake.health = 99999999.f;
	}

	// ----- Ui -----
	MUiPoolInput(Ed->mu);
	mu_begin(Ed->mu); {
		mu_Context *ctx = Ed->mu;
		if (Ed->selected && mu_begin_window(ctx, EntityTypeNames[Ed->selected->type], mu_rect(Data->canvas_size.x * 0.1f, Data->canvas_size.y * 0.1f, 150, 200))) {
			edit_entity(Ed->selected);
			mu_end_window(ctx);
		}
		if (Ed->debug_window) {
			if (mu_begin_window(ctx, "Debug window", mu_rect(Data->canvas_size.x * 0.1f, Data->canvas_size.y * 0.1f, 150, 200))) {
				mu_layout_row(ctx, 2, (int[]){0, -1}, 0);
				mu_label(ctx, "Tower infinity health");
				//mu_checkbox(ctx, "", &Ed->no_lose);
				MUiToggleButtonEx(ctx, &Ed->no_lose, 0);
				mu_end_window(ctx);
			}
			style_window(Ed->mu);
		}
		//process_frame(Ed->mu);
	} mu_end(Ed->mu);

	for (int i = 0; i < MU_CONTAINERPOOL_SIZE; i++) {
		mu_Container container = Ed->mu->containers[i];
		Rect rec = RectFromMu(container.rect);
		if (CheckCollisionPointRec(mouse_pos, rec)) return ;
	}

	// ----- Editor ----- 
	
	get_entity_under_mouse();

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (Ed->hovered) {
			Ed->selected = Ed->hovered;
			Ed->hovered = NULL;

			// If clicking on same entity that window was closed, open it back up
			mu_Container *cnt = mu_get_container(Ed->mu, EntityTypeNames[Ed->selected->type]); cnt->open = true;
		} else Ed->selected = NULL;
	}

}

void draw_editor()
{
	Color selected_highlight = BLACK;
	Color hover_highlight = PURPLE;
	if (Ed->selected) {
		DrawRectangleLinesEx(RecV2(Ed->selected->pos, Ed->selected->size), 1, selected_highlight);
	}
	if (Ed->hovered) {
		DrawRectangleLinesEx(RecV2(Ed->hovered->pos, Ed->hovered->size), 1, hover_highlight);
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


	mu_layout_row(ctx, 2, (int[]) {label_w, -1}, 0);
	mu_label(ctx, "Floor");
	mu_label(ctx, TextFormat("%d", e->floor));

	mu_layout_row(ctx, 3, (int[]) {label_w, width, -1}, 0);
	mu_label(ctx, "Position");
	mu_number_ex(ctx, &e->pos.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->pos.y, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_label(ctx, "Size");
	mu_number_ex(ctx, &e->size.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->size.y, 3, "%.1f", MU_OPT_ALIGNCENTER);

	width = body_width * 0.15;
	mu_layout_row(ctx, 6, (int[]) {label_w, width, width, width, width, -1}, 0);
	mu_label(ctx, "Color");
	u8_slider(ctx, &e->render.color.r, 0, 255);
	u8_slider(ctx, &e->render.color.g, 0, 255);
	u8_slider(ctx, &e->render.color.b, 0, 255);
	u8_slider(ctx, &e->render.color.a, 0, 255);
	mu_draw_rect(ctx, mu_layout_next(ctx), ColorToMu(e->render.color));
}

internal void get_entity_under_mouse() 
{
	V2 mouse_pos = GetMousePosition();

	{entitys_iterate(Data->level->entitys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;

		Rect e_rec = RecV2(e->pos, e->size);
		if (CheckCollisionPointRec(mouse_pos, e_rec)) {
			Ed->hovered = e;
			return ;
		}
	}}
	
	Ed->hovered = NULL;
}

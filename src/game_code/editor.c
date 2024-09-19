#include "game.h"
#include "./microui_exemple.c"

global GameData *Data = NULL;
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
	Data = data;
	Ed = &data->editor;
}

void update_editor()
{
	V2 mouse_pos = GetMousePosition();

	// ----- Ui -----
	MUiPoolInput(Ed->mu);
	mu_begin(Ed->mu); {
		mu_Context *ctx = Ed->mu;
		if (Ed->selected && mu_begin_window(ctx, "Entity", mu_rect(Data->canvas_size.x * 0.1f, Data->canvas_size.y * 0.1f, 150, 200))) {
			edit_entity(Ed->selected);
			mu_end_window(ctx);
		}
		style_window(Ed->mu);
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

	mu_label(ctx, EntityTypeNames[e->type]);
	i32 body_width = mu_get_current_container(ctx)->body.w;
	i32 width = body_width * 0.30;
	mu_layout_row(ctx, 3, (int[]) {50, width, -1}, 0);
	//mu_textbox_ex(ctx, "Position", 8, MU_OPT_ALIGNCENTER);
	mu_label(ctx, "Position");
	mu_number_ex(ctx, &e->pos.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->pos.y, 3, "%.1f", MU_OPT_ALIGNCENTER);

	mu_label(ctx, "Size");
	mu_number_ex(ctx, &e->size.x, 3, "%.1f", MU_OPT_ALIGNCENTER);
	mu_number_ex(ctx, &e->size.y, 3, "%.1f", MU_OPT_ALIGNCENTER);

	width = body_width * 0.15;
	mu_layout_row(ctx, 6, (int[]) {50, width, width, width, width, -1}, 0);
	mu_label(ctx, "Color");
	u8_slider(ctx, &e->color.r, 0, 255);
	u8_slider(ctx, &e->color.g, 0, 255);
	u8_slider(ctx, &e->color.b, 0, 255);
	u8_slider(ctx, &e->color.a, 0, 255);
	mu_draw_rect(ctx, mu_layout_next(ctx), ColorToMu(e->color));
}

internal void get_entity_under_mouse() 
{
	V2 mouse_pos = GetMousePosition();

	{entitys_iterate(Data->level->turrets) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;

		Rect e_rec = RecV2(e->pos, e->size);
		if (CheckCollisionPointRec(mouse_pos, e_rec)) {
			Ed->hovered = e;
			return ;
		}
	}}

	{entitys_iterate(Data->level->enemys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;

		Rect e_rec = RecV2(e->pos, e->size);
		if (CheckCollisionPointRec(mouse_pos, e_rec)) {
			Ed->hovered = e;
			return ;
		}
	}}

	{entitys_iterate(Data->level->projectiles) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;

		Rect e_rec = RecV2(e->pos, e->size);
		if (CheckCollisionPointRec(mouse_pos, e_rec)) {
			Ed->hovered = e;
			return ;
		}
	}}
	
	{entitys_iterate(Data->level->spawners) {
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

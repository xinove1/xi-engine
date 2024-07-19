#include "game.h"

static GameData *Data = NULL;
static GameEditor *E = NULL;

void	init_editor(GameData *data)
{
	TraceLog(LOG_INFO, "Initting Editor");
	Data = data;
	E = &data->editor;

	E->panel = CreateContainer((V2) {Data->canvas_size.x * 0.8f, Data->canvas_size.y * 0.3f}, 0, (UiConfig) {
			.alignment = UiAlignLeft,
			.font = (FontConfig) {
				.font = GetFontDefault(),
				.size = 10,
				.spacing = 1,
				.tint = BLACK,
				.tint_hover = RED,
			},
			.draw_container_bounds = true,
			.play_sound = false,
			.draw_selector = false,
			.take_key_input = false,
			.padding_row = 10,
			.padding_collumn = 3,
			.padding_element = 1.5f,
			.padding_border = 2,
			.color_background = YELLOW,
			.color_font = RED,
			.color_font_highlight = BLACK,
			.color_borders = BLACK,
	});
}

void	update_editor()
{
	V2	mouse_pos = V2Subtract(GetMousePosition(), Data->current_level->map_offset);
	E->selected_tile = (V2) {(int) mouse_pos.x / TILE, (int) mouse_pos.y / TILE}; //NOLINT
	if (E->selected_tile.x < 0 || E->selected_tile.x >= Data->current_level->map_sz.x || E->selected_tile.y < 0 || E->selected_tile.y >= Data->current_level->map_sz.y) {
		E->dragging = false;
		E->dragged_entity = NULL;
		return ;
	}
	
	if (!E->putting_new_entity && !E->dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		E->dragged_entity = get_map_entity(Data->current_level, E->selected_tile);
		if (E->dragged_entity) {
			E->dragging = true;
		} else { // Try Actuator
			E->dragged_entity = get_actuator(Data->current_level, E->selected_tile);
			if (E->dragged_entity) E->dragging = true;
		}
	}
	
	if (E->dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		if (!V2Compare(E->selected_tile, E->dragged_entity->pos)) {
			if (E->dragged_entity->type == EntityActuator && get_actuator(Data->current_level, E->selected_tile) == NULL) {
				E->dragged_entity->pos = E->selected_tile;
			} else if (E->dragged_entity->type != EntityActuator) {
				move_entity_swap(Data->current_level, E->dragged_entity, E->selected_tile);
			}
		}
		E->dragging = false;
		E->dragged_entity = NULL;
	} else if (E->dragging && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { // Cancel Dragging
		E->dragging = false;
		E->dragged_entity = NULL;
	}
}

void	draw_editor()
{
	{
		UiContainer *panel = &E->panel;
		UiBegin(panel);
		UiTitleBarEx(panel, panel->config, "Panel Test", panel->config.font, GRAY);
		UiText(panel, "dou", true);
		UiTextButton(panel, "oiii");
		UiTextButton(panel, "naoo");
		UiEnd(panel);
	}
	if (E->selected_tile.x < 0 || E->selected_tile.x >= Data->current_level->map_sz.x || E->selected_tile.y < 0 || E->selected_tile.y >= Data->current_level->map_sz.y) {
		return ;
	}

	// Draw Current Tile Under Cursor
	{
		V2	selected_pos = V2Add(V2Scale(E->selected_tile, TILE), Data->current_level->map_offset);
		Color	color = ColorAlpha(YELLOW, 0.7f);
		DrawRectangleLinesEx(RectV2(selected_pos, (V2) {TILE, TILE}), 1, color);
	}

	// Highlight Actuators
	// TODO  Better indicator that it is a Actuator bellow
	for (int i = 0; i < Data->current_level->actuators_count; i++) {
		Entity *act = &Data->current_level->actuators[i];
		if (get_map_pos(Data->current_level, act->pos) != -1) {
			Color color = ColorAlpha(act->color, 0.4f);
			DrawRectangleV(V2Add(V2Scale(act->pos, TILE), Data->current_level->map_offset), (V2) {TILE, TILE}, color);
		}
	}

	// Draw Preview of new position of dragged_entity
	if (E->dragging && !V2Compare(E->selected_tile, E->dragged_entity->pos)) {
		Color color = ColorAlpha(E->dragged_entity->color, 0.5f);
		DrawRectangleV(V2Add(V2Scale(E->selected_tile, TILE), Data->current_level->map_offset), (V2) {TILE, TILE}, color);
	}
}
#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif

internal b32 check_actuators(GameLevel *level);
internal b32 move_player(GameLevel *level, Entity *p, V2 dir);
internal b32 move_mixable(GameLevel *level, Entity *p, V2 dir);

global GameData  *Data = NULL;
global GameLevel *Level= NULL;
global Entity *Player= NULL;
global b32 WonLevel = false; // TODO  Temporary, change it

hot GameConfig init_pre_raylib(void **data)
{
	Data = calloc(1, sizeof(GameData));
	*data = Data;

	*Data = (GameData) {
		.canvas_size = (V2) {640, 360},
		.paused = false,
		.current_level = NULL,
		.menu_screen = false,
	};

	// Create sample Level
	{
		V2 map_size = {15, 10};
		GameLevel *level = create_level("Teste Level", map_size, Data->canvas_size);

		create_actuator(level, (Entity) {
			.type = EntityActuator,
			.color = RED,
			.pos = (V2) { 2, 2},
		});

		create_actuator(level, (Entity) {
			.type = EntityActuator,
			.color = RED,
			.pos = (V2) { 8, 2},
		});

		// NOTE  Player Entitys always need to be first to be created
		Player = create_entity(level, (Entity) {
			.type = EntityPlayer,
			.pos = (V2) { (i32) map_size.x / 2, (i32) map_size.y / 2}, // NOLINT para de reclamar do int meu irmao pqp
			.color = BLUE,
		});

		create_entity(level, (Entity) {
			.type = EntityMovable,
			.pos = { 3, 3 },
			.color = GREEN,
			});

		create_entity(level, (Entity) {
			.type = EntityMovable,
			.pos = { 5, 3 },
			.color = GREEN,
			});

		create_entity(level, (Entity) {
			.type = EntityMixable,
			.pos = { 7, 3 },
			.color = BLUE,
			});
		
		create_entity(level, (Entity) {
			.type = EntityMixable,
			.pos = { 7, 4 },
			.color = RED,
			});

		Level = level;
		Data->current_level = level;
	}

	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Sokaban",
		.window_flags = FLAG_WINDOW_RESIZABLE,
		.target_fps = 60,
	});
}

hot void init_pos_raylib() 
{
	init_editor(Data);

	Data->menu = UiCreateContainer((V2) {Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.3f}, 0, (UiConfig) {
			.alignment = UiAlignCentralized,
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
			.take_key_input = true,
			.padding_row = 10,
			.padding_collumn = 5,
			.color_background = YELLOW,
			.color_font = RED,
			.color_font_highlight = BLACK,
			.color_borders = BLACK,
	});
}

hot b32 update()
{
	assert(Data && Level && Player);
	if (Data->menu_screen) {
		return (false);
	}

	V2 dir = {0, 0};
	if (IsActionPressed(RIGHT)) {
		dir.x += 1;
	}
	if (IsActionPressed(LEFT)) {
		dir.x -= 1;
	}
	if (IsActionPressed(DOWN)) {
		dir.y += 1;
	}
	if (IsActionPressed(UP)) {
		dir.y -= 1;
	}
	Player->look_dir = dir;

	if (IsKeyPressed(KEY_U)) {
		exit(0);
		// print_entity(*Player);
		// print_map(Level);
		// print_level(Level);
	}

	// Resolve Collision
	if (dir.x != 0 || dir.y != 0) {
		move_player(Level, Player, dir);
	}

	// Run Actuators
	WonLevel = check_actuators(Level);

	update_editor();
	return (false);
}

hot void draw()
{
	if (Data->menu_screen) {
		UiContainer *c = &Data->menu;
		UiBegin(c);

		UiText(c, "Sokaban da silva", true);
		
		if (UiTextButton(c, "Play")) {
			Data->menu_screen = false;
		}

		if (UiTextButton(c, "Options")) {
			printf("Optiooooons\n");
		}

		UiEnd(c);
		return ;
	}

	// Draw Background outside of map
	DrawRectangle(0, 0, Data->canvas_size.x, Data->canvas_size.y, BLACK);

	// Draw Map borders & backgroud/ground of map
	DrawRectangle(Level->map_offset.x, Level->map_offset.y, Level->map_sz.x * TILE, Level->map_sz.y * TILE, WHITE);

	// Draw Actuators
	for (i32 i = 0; i < MAX_ACTUATORS; i++) {
		Entity e = Level->actuators[i];
		if (e.type == EntityEmpty) break;
		DrawRectangleV(V2Add(V2Scale(e.pos, TILE), Level->map_offset), (V2) {TILE, TILE}, e.color);
	}

	// Draw Entitys
	for (i32 i = 0; i < Level->entity_count; i++) {
		Entity e = Level->entitys[i];
		if (e.type == EntityEmpty) continue ;
		DrawRectangleV(V2Add(V2Scale(e.pos, TILE), Level->map_offset), (V2) {TILE, TILE}, e.color);
	}

	if (WonLevel) {
		V2 canvas_middle = V2Scale(Data->canvas_size, 0.5f);
		DrawTextEx(GetFontDefault(), "You Won!!", canvas_middle, 12, 1, RED);
	}

	// Draw editor
	draw_editor();
}

hot void pre_reload()
{
}

hot void pos_reload(void *data)
{
	Data = data;
	Level = Data->current_level;
	Player = get_entity(Data->current_level, 0);
	assert(Player->type == EntityPlayer); // Player Entity should awalys be the first on the entitys array
	
	init_editor(data);
}

GameFunctions game_init_functions()
{
	return (GameFunctions) {
		.init_pre_raylib = &init_pre_raylib,
		.init_pos_raylib = &init_pos_raylib,
		.update = &update,
		.draw = &draw,
		.pre_reload = &pre_reload,
		.pos_reload = &pos_reload,
	};
}

internal b32 move_player(GameLevel *level, Entity *p, V2 dir)
{
	V2 where = V2Add(p->pos, dir);

	Entity *e = get_map_entity(level, where);
	if (e) {
		switch (e->type) {
			case EntityMovable:
				move_entity(level, e, V2Add(e->pos, dir));
			break ;
			case EntityMixable:
				move_mixable(level, e, dir);
			break ;
			default:
		
			break ;
		}
	}

	return (move_entity(Level, p, where));
}

internal b32 move_mixable(GameLevel *level, Entity *p, V2 dir)
{
	V2 where = V2Add(p->pos, dir);

	Entity *e = get_map_entity(level, where);
	if (e && e->type == EntityMixable) {
		e->color = GetColor(ColorToInt(e->color) | ColorToInt(p->color));
		delete_entity(level, p);
	} 

	return (move_entity(level, p, where));;
}

internal b32 check_actuators(GameLevel *level)
{
	b32 all_set = true;

	for (i32 i = 0; i < MAX_ACTUATORS; i++) {
		Entity e = Level->actuators[i];
		if (e.type == EntityEmpty) break;

		i32 something_on_top = get_map_pos(level, e.pos);
		if (something_on_top == -1) {
			all_set = false;
		}
	}

	return (all_set);
}

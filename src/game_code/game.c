#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif

GameLevel *create_level(GameData *data, size floors);
internal b32 update_game(void);
internal b32 update_input(void);
internal b32 update_ui(void);

GameData  *Data = NULL;
global GameLevel *Level= NULL;

// NOTE  for web build
// TODO  Add option on menu to not pause game on leaving canvas focus
void pause_game(void) 
{
	Data->paused = true;
}

hot GameConfig init_pre_raylib(void **data)
{
	Data = calloc(1, sizeof(GameData));
	*data = Data;

	*Data = (GameData) {
		.canvas_size = (V2) {640, 360},
		//.canvas_size = (V2) {1280, 720},
		.particles = {0},
		.paused = false,
		.level = NULL,
		.game_speed = 1,
		.menu_screen = false,
	};
	Data->ui.mu = calloc(1, sizeof(mu_Context));
	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Cake Defense",
		.window_flags = FLAG_WINDOW_RESIZABLE,
		.target_fps = 60,
	});
}

hot void init_pos_raylib(void) 
{
	Image image = LoadImage("assets/monogram-bitmap.png");
	Data->assets.font = LoadFontFromImageSheet(image, Vec2(6, 12), 32);
	UnloadImage(image);
	Data->assets.sheet_ui = LoadTexture("assets/ui_sheet.png");
	Data->assets.sheet_ant = LoadTexture("assets/ant_sheet.png");
	Data->ui.paused = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 0);
	Data->ui.speed = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 2);
	Data->ui.next_wave = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 8);
	Data->ui.buy_turret = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 7);
	MUiInit(Data->ui.mu, &Data->assets.font, Data->canvas_size);

	Data->menu = XUiCreateContainer((V2) {Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.3f}, 0, (UiConfig) {
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
	Data->level = create_level(Data, 5);
	Level = Data->level;
	init_editor(Data);
}

hot void pre_reload(void)
{

}

hot void pos_reload(void *data)
{
	Data = data;
	Level = Data->level;
	pos_reload_editor(Data);
	// size offset = offset_of(GameLevel, entitys);
	// printf("offset: %ld \n", offset);
	// printf("sizeof entity: %ld \n", sizeof(Entity));
}


hot b32 update(void)
{
	assert(Data && Level);

	update_editor();
	update_ui();

	if (Data->menu_screen || Data->lost) {
		return (false);
	}

	if (Level->cake.health <= 0) {
		Data->lost = true;
	}

	update_input();

	if (Data->paused) return (false);

	for (i32 i = 0; i < Data->game_speed; i++) {
		if (update_game()) return (true);
	}

	return (false);
}

internal b32 update_ui(void) 
{
	i32 window_flag = MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_AUTOSIZE | MU_OPT_NORESIZE;
	i32 texture_button = 22;
	mu_begin(Data->ui.mu); {
		mu_Context *ctx = Data->ui.mu;
		if (mu_begin_window_ex(ctx, "PauseUi", MuRec(0, 0, GetCanvasRec().width + 1, 42), window_flag ^ MU_OPT_AUTOSIZE)) {
			mu_Rect cnt_rect = mu_get_current_container(ctx)->rect;
			i32 w = mu_get_current_container(ctx)->rect.w;
			mu_layout_row(ctx, 3, (const int[]) {w * 0.25, w * 0.50, w * 0.25}, 24);

			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 3, (const int[]) {texture_button, texture_button, texture_button}, texture_button);

			// Buttons
			mu_layout_set_next(ctx, mu_rect(0, (cnt_rect.h - texture_button) / 4, texture_button, texture_button), 1);
			mu_tooltip(ctx, (Data->paused) ? "Unpause game" : "Pause Game");
			Data->ui.paused.frame = Data->paused ? 1 : 0;
			if (MUiTextureButton(ctx, &Data->ui.paused, MU_OPT_ALIGNCENTER)) {
				Data->paused = Data->paused ? false : true;
			}
			mu_layout_set_next(ctx, mu_rect(texture_button, (cnt_rect.h - texture_button) / 4, texture_button, texture_button), 1);
			mu_tooltip(ctx, "Change the game speed");
			if (MUiTextureButton(ctx, &Data->ui.speed, MU_OPT_ALIGNCENTER)) {
					Data->game_speed += 1;
					if (Data->game_speed > MAX_GAME_SPEED) {
						Data->game_speed = 1;
					}
				Data->ui.speed.frame = Data->game_speed + 1;
			}
			mu_layout_set_next(ctx, mu_rect(texture_button * 2, (cnt_rect.h - texture_button) / 4, texture_button, texture_button), 1);
			mu_tooltip(ctx, "Begin Wave Early");
			if (MUiTextureButton(ctx, &Data->ui.next_wave, MU_OPT_ALIGNCENTER)) {
				start_wave(Data->level);
			}
			mu_layout_end_column(ctx);

			mu_end_window(ctx);
		}
		if (Level->turret_selected) { 
			Turret *t = Level->turret_selected;
			V2 size = {texture_button * 2.5, texture_button * 2.5};
			V2 pos = t->pos;
			f32 padding = 5;
			i32 options = MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NOHOLD_POS | MU_OPT_NOHOLD_SIZE;
			if (CheckCollisionRecs(GetCanvasRec(), RecV2(Vec2(pos.x, pos.y - size.y - padding), size))) {
				pos.y -= size.y - padding;
			} else {
				pos.y += size.y + padding;
			}

			mu_push_id(ctx, t, sizeof(Turret));
			if (t->type == EntityTurret) {
				// mu_layout_width(ctx, 0);
				// if (mu_begin_window_ex(ctx, "Upgrade Tower", MuRecV2(pos, size), options)) {
				// 	mu_end_window(ctx);
				// }
			} 
			else if (t->type == EntityTurretSpot) {
				if (mu_begin_window_ex(ctx, "Buy Turret", MuRecV2(pos, size), options)) {
				mu_layout_row(ctx, 1, (const int[]) {texture_button * 2}, texture_button * 2);
				mu_tooltip(ctx, "Buy Turret");
				if (MUiTextureButton(ctx, &Data->ui.buy_turret, MU_OPT_ALIGNCENTER)) {
						// TODO  Kill any Enemy that's on turret spot
						*t = create_turret((Turret) {
							.type = EntityTurret,
							.pos = t->pos,
							.size = t->size,
							.render.tint = RED,
							.fire_rate = 0.1,
							.damage = 4,
							.range = 30,
							.health = 100,
							.floor = t->floor,
						});
					}
					mu_end_window(ctx);
				}
			}
			else {
				TraceLog(LOG_WARNING, "Turret selected does not have a valid type.");
			}
			mu_pop_id(ctx);
		}
		if (mu_begin_window_ex(ctx, "GameInfo", MuRec(GetCanvasRec().width * 0.35, 10, 300, 50), window_flag )) {
			
			mu_layout_row(ctx, 2, (const int[]) {150, 0}, 24);

			mu_end_window(ctx);
		}

	} mu_end(Data->ui.mu);

	return (false);
}

internal b32 update_input(void) 
{
	MUiPoolInput(Data->ui.mu);

	V2 input_dir = {0, 0};
	if (IsActionPressed(RIGHT)) {
		input_dir.x += 1;
	}
	if (IsActionPressed(LEFT)) {
		input_dir.x -= 1;
	}
	if (IsActionPressed(DOWN)) {
		input_dir.y += 1;
	}
	if (IsActionPressed(UP)) {
		input_dir.y -= 1;
	}

	// Mouse turret Selection
	if (!Level->turret_selected || (Level->turret_selected && !MUiIsMouseInsideContainer(Data->ui.mu))) { 
		local i32 frame_count = 0;
		frame_count++;
		if (frame_count >= 5) {
			V2 mouse_pos = GetMousePosition();
			Level->turret_hovered = NULL;
			{da_iterate(Level->turrets, TurretDa) {
				Turret *e = iterate_get();
				if (e->type == EntityEmpty) continue;
				if (CheckCollisionPointRec(mouse_pos, RecV2(e->pos, e->size))) {
					Level->turret_hovered = e;
					break ;
				}
			}}
			frame_count = 0;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if (!Level->turret_hovered || Level->turret_hovered == Level->turret_selected) {
				Level->turret_selected = NULL;
			} else if (Level->turret_hovered) {
				Level->turret_selected = Level->turret_hovered;
			}
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				Level->turret_selected = NULL;
				Level->turret_hovered = NULL;
		}
	}
	
	#ifdef BUILD_DEBUG
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		for (i32 j = 0; j < 100; j++){
			create_particle( 
				.dir = Vec2(GetRandf32(-1, 1), -1),
				.velocity = GetRandf32(50, 300),
				.pos = V2Add(GetMousePosition(), Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1))),
				.size = Vec2v(2),
				.duration = GetRandf32(0.3f, 1),
				.color_initial = ColA(255, 0, 0, 255),
				.color_end = ColA(255, 0, 0, 0),
			);
		}
	}
	if (IsKeyPressed(KEY_U)) {
		exit(0);
	}
	#endif

	return (false);
}

internal b32 update_game(void) 
{

	// ----------- Particles -----------
	
	apply_func_entitys(Level, update_entity_veffects);


	// ----------- Update Entitys ----------- 
	
	apply_func_entitys(Level, update_entity_animations);

	update_wave_manager(Level);

	// ----------- Enemys ----------- 
	{da_iterate(Level->enemys, EnemyDa) {
		Enemy *e = iterate_get();
		iterate_check_entity(e, EntityEnemy);
		update_enemy(Level, e);
	}}

	// ----------- Turrets ----------- 
	{da_iterate(Level->turrets, TurretDa) {
		Turret *e = iterate_get();
		iterate_check_entity(e, EntityTurret);
		update_turret(Level, e);
	}}

	// ----------- Projectiles ----------- 
	{da_iterate(Level->projectiles, ProjectileDa) {
		Projectile *e = iterate_get();
		iterate_check_entity(e, EntityProjectile);
		update_projectile(Level, e);
	}}
	return (false);
}

hot void draw(void)
{
	//DrawRectangle(0, 0, Data->canvas_size.x, Data->canvas_size.y, BLACK);
	ClearBackground((Color){130,200,229, 255});
	apply_func_entitys(Level, render_entity);

	render_env_sprites(Level->enviromnent_sprites, length_of(Level->enviromnent_sprites));

	for (i32 i = 0; i < count_of(Data->particles); i++) {
		if (Data->particles[i].type == ParticleEmpty) continue;
		Particle *p = &Data->particles[i];
		p->pos = V2Add(p->pos, V2Scale(p->dir, p->velocity * GetFrameTime()));
		p->duration_count += GetFrameTime();
		if (p->duration_count >= p->duration) {
			*p = (Particle) { 0 };
			continue;
		}
		render_particle(Data->particles[i]);
	}

	if (Level->turret_selected) {
		Turret *t = Level->turret_selected;
		Rect rec = RecV2(V2Add(t->pos, t->render.pos), t->render.size);
		DrawRectangleLinesEx(rec, 1, BLACK); // TODO Fix Color
	}

	if (Level->turret_hovered && Level->turret_hovered != Level->turret_selected) {
		Turret *t = Level->turret_hovered;
		Rect rec = RecV2(V2Add(t->pos, t->render.pos), t->render.size);
		DrawRectangleLinesEx(rec, 1, PURPLE); // TODO Fix Color
	}

	// ---- Text ---
	MUiRender(Data->ui.mu);
	Font font = Data->assets.font;
	i32 font_size = Data->assets.font.baseSize;
	f32 font_spacing = 2;
	Color font_color = PURPLE;
	{ 
		const cstr *health_text = TextFormat("Cake health: %.f/%.f", Level->cake.health, Level->cake.health_max);
		V2 cake_size = MeasureTextEx(font, health_text, font_size, font_spacing);
		V2 cake_pos = Vec2(Data->canvas_size.x * 0.5f - cake_size.x * 0.5f, 12);
		DrawTextEx(font, health_text, V2i32(cake_pos), font_size, font_spacing, font_color);

		{
			const cstr *text = TextFormat("Wave: %d", Level->wave_manager.wave);
			V2 size = MeasureTextEx(font, text, font_size, font_spacing);
			i32 c = cake_pos.x + cake_size.x;
			c +=  (Data->canvas_size.x - c) * 0.5f;
			//DrawCircleV(Vec2(c, 10), 5, RED);
			V2 pos = Vec2(c - size.x * 0.5f, cake_pos.y);
			DrawTextEx(font, text, V2i32(pos), font_size, font_spacing, font_color);
		}
		if (Level->wave_manager.time_count != 0) { 
			const cstr *text = TextFormat("Next wave in: %.f", Level->wave_manager.time_until_next_wave - Level->wave_manager.time_count);
			V2 size = MeasureTextEx(font, text, font_size, font_spacing);
			V2 pos = Vec2(Data->canvas_size.x * 0.5f - size.x * 0.5f, cake_pos.y + cake_size.y + 5);
			DrawTextEx(font, text, V2i32(pos), font_size, font_spacing, font_color);
		}
	}

	if (Data->lost) {
		const cstr *text = TextFormat("You Lost!");
		V2 size = MeasureTextEx(font, text, font_size, font_spacing);
		V2 pos = Vec2(Data->canvas_size.x * 0.5f - size.x * 0.5f, Data->canvas_size.y * 0.5f);
		DrawTextEx(Data->assets.font, text, pos, font_size, font_spacing, BLACK);
	}

	#ifdef BUILD_DEBUG 
		for (size i = 0; i < Level->floors_count * 2; i++) {
			V2 p = Level->wave_manager.locations[i].point;
			DrawCircleV(p, 2, GRAY);
		}
	#endif

	// ---- Ui -----

	if (Data->menu_screen) {
		UiContainer *c = &Data->menu;
		XUiBegin(c);

		XUiText(c, "Tower Defense da silva", true);
		
		if (XUiTextButton(c, "Play")) {
			Data->menu_screen = false;
		}

		if (XUiTextButton(c, "Options")) {
			printf("Optiooooons\n");
		}

		XUiEnd(c);
		return ;
	}


	draw_editor();
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

GameLevel *create_level(GameData *data, size floors) 
{
	GameLevel *level = calloc(1, sizeof(GameLevel));
	level->floors_count = floors;

	f32 floor_height = TILE_TURRET_SIZE;
	f32 floor_padding = 8;
	f32 turret_width = TILE_TURRET_SIZE;
	f32 tower_width = 64;
	f32 tower_health = 400;
	f32 ground_height = 50;

	V2 canvas = data->canvas_size;
	V2 canvas_middle = Vec2(canvas.x * 0.5f, canvas.y * 0.5f);
	V2 tower_size = Vec2(tower_width, (floor_height * floors) + (floor_padding * floors));
	V2 tower_pos = Vec2(canvas_middle.x - (tower_size.x * 0.5f), canvas.y - (tower_size.y + ground_height));
	level->cake = (GenericEntity) {
		.type = EntityCake,
		.render.size = tower_size,
		.render.tint = PURPLE,
		.pos = tower_pos,
		.size = tower_size,
		.health = tower_health,
		.health_max = tower_health,
	};

	// Enviroment
	Texture2D sprite_ground = {0};
	EnvSprite env_sprite_ground = (EnvSprite) {
		.type = EnvSpriteStatic,
		.pos = {0, Data->canvas_size.y - ground_height},
		.sprite = CreateSprite(sprite_ground, .size = {data->canvas_size.x, ground_height}, .tint = BROWN),
	};
	create_env_sprite(level->enviromnent_sprites, length_of(level->enviromnent_sprites), env_sprite_ground);

	// Init Entitys
	size max_turrets = floors * 2;
	size max_projectiles = 400;
	size max_enemys = floors * 100 * 2;
	da_init_and_alloc(level->turrets, max_turrets, sizeof(Turret));
	da_init_and_alloc(level->projectiles, max_projectiles, sizeof(Projectile));
	da_init_and_alloc(level->enemys, max_enemys, sizeof(Enemy));


	// Init Wave Manager
	{
		level->wave_manager = (WaveManager) { 0 };
		level->wave_manager.floor_limit = 1;
		level->wave_manager.time_until_next_wave = 5;
		level->wave_manager.packets_max = max_enemys;
		level->wave_manager.packets = calloc(level->wave_manager.packets_max, sizeof(SpawnPacket));
		level->wave_manager.locations = calloc(floors * 2, sizeof(SpawnLocation));
		// Spawn Locations
		size max_locations = floors * 2;
		for (i32 i = 0; i < max_locations; i++) {
			V2 pos = {0, canvas.y - ground_height};
			f32 padding = 5; // Padding from screen border
			i32 floor = 0;
			if (i < max_locations * 0.5f) {
				floor += i;
				pos.x = padding;
			}
			else { // Right side
				floor += i - max_locations/2.f;
				pos.x = canvas.x - padding;
			}
			if (floor == 0) {
				pos.y -= (floor_height * 0.5f) * (floor + 1);
			} else {
				pos.y -= (floor_padding + floor_height) * floor + (floor_height * 0.5f);
			}
			
			level->wave_manager.locations[i].point = pos; // TODO  Get middle of floor instead of ceiling
			level->wave_manager.locations[i].floor = floor;
		}
	}

	// Turrets
	for (i32 i = 0; i < max_turrets; i++) {
		V2 pos = {0, canvas.y - ground_height};
		i32 floor = 0;
		// Left side
		if (i < max_turrets * 0.5f) {
			floor += i;
			pos.x = tower_pos.x -  turret_width;
		}
		else { // Right side
			floor += i - max_turrets/2.f;
			pos.x = tower_pos.x + tower_width;
		}
		if (floor == 0) {
			pos.y -= floor_height * (floor + 1);
		} else {
			pos.y -= (floor_padding + floor_height) * floor + floor_height;
		}

		if (floor > floors -2 ) {
			spawn_turret(level, create_turret_prefab(TURRET_SPOT), floor, pos);
		} 
		else {
			if (GetRandomValue(0, 100) > 80) {
				spawn_turret(level, create_turret_prefab(TURRET_MACHINEGUN), floor, pos);
			} else {
				spawn_turret(level, create_turret_prefab(TURRET_BASIC), floor, pos);
			}
		}
	}

	return (level);
}

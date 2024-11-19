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
	printf("sheet_ant size: %d, %d \n", Data->assets.sheet_ant.width, Data->assets.sheet_ant.height);
	Data->ui.paused = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 0);
	Data->ui.speed = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 2);
	MUiInit(Data->ui.mu, &Data->assets.font);

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

	if (Data->menu_screen || Data->lost) {
		return (false);
	}

	if (Level->cake.health <= 0) {
		Data->lost = true;
	}

	update_input();
	update_ui();

	if (Data->paused) return (false);

	for (i32 i = 0; i < Data->game_speed; i++) {
		if (update_game()) return (true);
	}

	return (false);
}

internal b32 update_ui(void) 
{
	mu_begin(Data->ui.mu); {
		mu_Context *ctx = Data->ui.mu;
		if (mu_begin_window_ex(ctx, "PauseUi", MuRec(10, 10, 80, 30), MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_AUTOSIZE | MU_OPT_NORESIZE)) {
			mu_layout_row(ctx, 3, (const int[]) {24, 24, 100}, 24);
			mu_tooltip(ctx, (Data->paused) ? "Unpause game" : "Pause Game");
			Data->ui.paused.frame = Data->paused ? 1 : 0;
			if (MUiTextureButton(ctx, &Data->ui.paused, MU_OPT_ALIGNCENTER)) {
				Data->paused = Data->paused ? false : true;
			}
			mu_tooltip(ctx, "Change the game speed");
			if (MUiTextureButton(ctx, &Data->ui.speed, MU_OPT_ALIGNCENTER)) {
					Data->game_speed += 1;
					if (Data->game_speed > MAX_GAME_SPEED) {
						Data->game_speed = 1;
					}
				Data->ui.speed.frame = Data->game_speed + 1;
			}
			mu_tooltip(ctx, "Begin Wave Early");
			if (mu_button_ex(ctx, "BeginWaveEarly", 0, 0)) {
				start_wave(Data->level);
			}
			mu_end_window(ctx);
		}
		if (Level->turret_selected) { 
			Turret *t = Level->turret_selected;
			V2 size = {100, 50};
			V2 pos = t->pos;
			f32 padding = 5;
			i32 options = MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NOHOLD_POS | MU_OPT_NOHOLD_SIZE;
			if (CheckCollisionRecs(GetCanvasRec(), RecV2(Vec2(pos.x, pos.y - size.y - padding), size))) {
				pos.y -= size.y - padding;
			} else {
				pos.y += size.y + padding;
			}
			if (t->type == EntityTurret) {
				if (mu_begin_window_ex(ctx, "Upgrade Tower", MuRecV2(pos, size), options)) {
					mu_end_window(ctx);
				}
			} 
			else if (t->type == EntityTurretSpot) {
				if (mu_begin_window_ex(ctx, "Buy turret", MuRecV2(pos, size), options)) {
					mu_layout_row(ctx, 1, (const int[]) {-1}, -1);
					if (mu_button_ex(ctx, "Buy Basic Turret", 0, 0)) {
						// TODO  Kill any Enemy that's on turret spot
						*t = create_turret((Turret) {
							.type = EntityTurret,
							.pos = t->pos,
							.size = t->size,
							.render.tint = BLACK,
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

		if (e->health <= 0 ) {
			e->type = EntityEmpty;
			continue;
		}

		V2 dir = V2DirTo(e->pos, Vec2(Data->canvas_size.x * 0.5f, e->pos.y));
		GenericEntity *target = (GenericEntity *)enemy_get_turret(Level->turrets, e->floor, dir.x);
		if (!target) target = &Level->cake; // Attack Main tower instead

		if (entity_in_range((GenericEntity *)e, (GenericEntity *)target, e->range)) {
			e->attack_rate_count += GetFrameTime();
			if (e->attack_rate_count >= e->attack_rate) {
				e->attack_rate_count = 0;
				damage_entity(Level, target, e->damage);
			}
		} else { // Move Towards Turret
			e->pos = V2Add(e->pos, V2Scale(dir, e->speed * GetFrameTime()));
		}
	}}


	// ----------- Turrets ----------- 
	{da_iterate(Level->turrets, TurretDa) {
		Turret *e = iterate_get();
		iterate_check_entity(e, EntityTurret);

		if (e->health <= 0) {
			e->type = EntityEmpty;
			continue;
		}

		e->fire_rate_count += GetFrameTime();
		if (e->fire_rate && e->fire_rate_count >= e->fire_rate) {
			e->fire_rate_count = 0;
			Enemy *target = turret_get_target(Level->enemys, *e, 1); // TODO  Add floor range to turret
			if (target) {
				V2 p = V2Add(e->pos, V2Scale(e->size, 0.5));
				spawn_projectile(p, target->pos, .size = Vec2(2, 2), .targeting = EntityEnemy, .speed = 400, .damage = 10); 
			}
		}
	}}

	// ----------- Projectiles ----------- 
	{da_iterate(Level->projectiles, ProjectileDa) {
		Projectile *e = iterate_get();
		iterate_check_entity(e, EntityProjectile);

		if (e->health <= 0) {
			// TODO  Move this to after hit a enemy an splatter the particles in ther direction of the collision
			for (i32 i = 0; i < 10; i++) {
				create_particle( 
					.dir = Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1)),
					.velocity = GetRandf32(50, 200),
					.pos = V2Add(e->pos, Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1))),
					.size = Vec2v(1),
					.duration = GetRandf32(0.1f, 0.4f),
					.color_initial = ColA(255, 0, 0, 255),
					.color_end = ColA(255, 0, 0, 0),
				);
			}
			e->type = EntityEmpty;
		}

		e->pos = V2Add(e->pos, V2Scale(e->dir, e->speed * GetFrameTime()));
		if (!CheckCollisionRecs(RecV2(V2Zero(), Data->canvas_size), RecV2(e->pos, e->size))) {
			e->type = EntityEmpty;
			continue ;
		}

		Rect rec = RecV2(e->pos, e->size);
		if (e->targeting == EntityTurret) {
			{da_iterate(Level->turrets, TurretDa){
				Turret *turret = iterate_get();
				iterate_check_entity(turret, EntityTurret);
				if (CheckCollisionRecs(rec, RecV2(turret->pos, turret->size))) {
					damage_entity(Level, (GenericEntity* )turret, e->damage);
					e->health -= turret->health;
					continue;
				}
			}}
		}
		if (e->targeting == EntityEnemy) {
			{da_iterate(Level->enemys, EnemyDa){
				Enemy *enemy = iterate_get();
				iterate_check_entity(enemy, EntityEnemy);
				if (CheckCollisionRecs(rec, RecV2(enemy->pos, enemy->size))) {
					damage_entity(Level, (GenericEntity* )enemy, e->damage);
					e->health -= enemy->health;
					continue;
				}
			}}
		}
	}}
	return (false);
}

hot void draw(void)
{
	//DrawRectangle(0, 0, Data->canvas_size.x, Data->canvas_size.y, BLACK);

	apply_func_entitys(Level, render_entity);

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
	Font font = Data->assets.font;
	i32 font_size = Data->assets.font.baseSize;
	f32 font_spacing = 2;
	{ 
		const cstr *health_text = TextFormat("Cake health: %.f/%.f", Level->cake.health, Level->cake.health_max);
		V2 cake_size = MeasureTextEx(font, health_text, font_size, font_spacing);
		V2 cake_pos = Vec2(Data->canvas_size.x * 0.5f - cake_size.x * 0.5f, 12);
		DrawTextEx(font, health_text, V2i32(cake_pos), font_size, font_spacing, RED);

		{
			const cstr *text = TextFormat("Wave: %d", Level->wave_manager.wave);
			//i32 size = MeasureText(text, 10);
			V2 pos = Vec2(cake_pos.x + cake_size.x + (font_spacing * 3), cake_pos.y);
			DrawTextEx(font, text, V2i32(pos), font_size, font_spacing, RED);
		}
		if (Level->wave_manager.time_count != 0) { 
			const cstr *text = TextFormat("Next wave in: %.f", Level->wave_manager.time_until_next_wave - Level->wave_manager.time_count);
			V2 size = MeasureTextEx(font, text, font_size, font_spacing);
			V2 pos = Vec2(Data->canvas_size.x * 0.5f - size.x * 0.5f, cake_pos.y + cake_size.y + 5);
			DrawTextEx(font, text, V2i32(pos), font_size, font_spacing, RED);
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

	MUiRender(Data->ui.mu);
	
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

	f32 floor_height = 32;
	f32 floor_padding = 8;
	f32 turret_width = 32;
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
		f32 fire_rate = GetRandf32(0.2, 1);
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
			spawn_turret(level, create_turret((Turret) {
				.type = EntityTurretSpot,
				.pos = pos,
				.size = Vec2(turret_width, floor_height),
				.render.tint = ColorAlpha(GRAY, 0.05),
				.fire_rate = fire_rate,
				.damage = 4,
				.range = 30,
				.health = 100,
				.floor = floor,
			}));
		} 
		else {
			spawn_turret(level, create_turret((Turret) {
				.type = EntityTurret,
				.pos = pos,
				.size = Vec2(turret_width, floor_height),
				.render.tint = RED,
				.fire_rate = fire_rate,
				.damage = 4,
				.range = 30,
				.health = 100,
				.floor = floor,
			}));
		}
	}

	return (level);
}

#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif

GameLevel *create_level(GameData *data, size floors);

GameData  *Data = NULL;
global GameLevel *Level= NULL;

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
		.menu_screen = false,
	};
	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Cake Defense",
		.window_flags = FLAG_WINDOW_RESIZABLE,
		.target_fps = 60,
	});
}

hot void init_pos_raylib(void) 
{
	Data->level = create_level(Data, 5);
	Level = Data->level;

	init_editor(Data);

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

	update_entity_veffects(&Level->cake);
	{entitys_iterate(Level->entitys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;
		update_entity_veffects(e);
	}}

	// ----------- Input -----------
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

	if (IsKeyPressed(KEY_U)) {
		exit(0);
	}
	
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

	// ----------- Update Entitys ----------- 
	
	update_wave_manager(Level);

	// ----------- Enemys ----------- 
	{entitys_iterate(Level->enemys) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityEnemy);

		if (entity_died(Level, e)) continue;

		V2 dir = V2DirTo(e->pos, Vec2(Data->canvas_size.x * 0.5f, e->pos.y));
		Entity *target = enemy_get_turret(Level->turrets, e->floor, dir.x);
		if (!target) target = &Level->cake; // Attack Main tower instead

		if (EntityInRange(e, target, e->enemy.range)) {
			e->enemy.attack_rate_count += GetFrameTime();
			if (e->enemy.attack_rate_count >= e->enemy.attack_rate) {
				e->enemy.attack_rate_count = 0;
				damage_entity(Level, target, e->enemy.damage);
			}
		} else { // Move Towards Turret
			e->pos = V2Add(e->pos, V2Scale(dir, e->enemy.speed * GetFrameTime()));
		}
	}}


	// ----------- Turrets ----------- 
	{entitys_iterate(Level->turrets) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityTurret);

		if (entity_died(Level, e)) continue;

		e->turret.fire_rate_count += GetFrameTime();
		if (e->turret.fire_rate && e->turret.fire_rate_count >= e->turret.fire_rate) {
			e->turret.fire_rate_count = 0;
			Entity *target = turret_get_target(Level->enemys, *e, 1);
			if (target) {
				V2 p = V2Add(e->pos, V2Scale(e->size, 0.5));
				push_entity(&Level->projectiles, create_projectile(p, target->pos, .size = Vec2(2, 2), .targeting = EntityEnemy, .speed = 400, .damage = 10)); // NOLINT
			}
		}
	}}

	// ----------- Projectiles ----------- 
	{entitys_iterate(Level->projectiles) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityProjectile);

		if (entity_died(Level, e)) continue;

		e->pos = V2Add(e->pos, V2Scale(e->bullet.dir, e->bullet.speed * GetFrameTime()));
		if (!CheckCollisionRecs(RecV2(V2Zero(), Data->canvas_size), RecV2(e->pos, e->size))) {
			e->type = EntityEmpty;
			continue ;
		}

		EntityDa check_against = {0};

		if (e->bullet.targeting == EntityTurret) check_against = Level->turrets;
		if (e->bullet.targeting == EntityEnemy) check_against = Level->enemys;

		if (check_against.capacity <= 0) continue;
		Entity *coll = check_collision(RecV2(e->pos, e->size), check_against);
		if (coll && coll->type == EntityEnemy) {
			damage_entity(Level, coll, e->bullet.damage);
			e->health -= coll->health;
			continue;
		}
	}}
	return (false);
}

hot void draw(void)
{
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

	// Background color
	//DrawRectangle(0, 0, Data->canvas_size.x, Data->canvas_size.y, BLACK);

	render_entity(&Level->cake);
	{entitys_iterate(Level->entitys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;
		render_entity(e);
	}}

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

	#ifdef BUILD_DEBUG 
		for (size i = 0; i < Level->floors_count * 2; i++) {
			V2 p = Level->wave_manager.locations[i].point;
			DrawCircleV(p, 2, GRAY);
		}
		// for (size i = 0; i < Level->floors_count; i++) {
		// 	V2 p1 = Level->wave_manager.locations[i].point;
		// 	V2 p2 = Level->wave_manager.locations[i + Level->floors_count].point;
		// 	DrawLineV(p1, p2, PURPLE);
		// }
	#endif

	// Texts
	{ 
		const cstr *cake_health = TextFormat("Cake health: %.f/%.f", Level->cake.health, Level->cake.health_max);
		i32 cake_size = MeasureText(cake_health, 10);
		V2 cake_pos = Vec2(Data->canvas_size.x * 0.5f - cake_size * 0.5f, 12);
		DrawText(cake_health, cake_pos.x, cake_pos.y, 10, RED);

		{
			const cstr *text = TextFormat("Wave: %d", Level->wave_manager.wave);
			i32 size = MeasureText(text, 10);
			V2 pos = Vec2(cake_pos.x + cake_size + 6, cake_pos.y);
			DrawText(text, pos.x, pos.y, 10, RED);
		}
		if (Level->wave_manager.time_count != 0) { 
			const cstr *text = TextFormat("Next wave in: %.f", Level->wave_manager.time_until_next_wave - Level->wave_manager.time_count);
			i32 size = MeasureText(text, 10);
			V2 pos = Vec2(Data->canvas_size.x * 0.5f - size * 0.5f, 24);
			DrawText(text, pos.x, pos.y, 10, RED);
		}
	}


	if (Data->lost) {
		const cstr *text = TextFormat("You Lost!");
		i32 size = MeasureText(text, 20);
		V2 pos = Vec2(Data->canvas_size.x * 0.5f - size * 0.5f, Data->canvas_size.y * 0.5f);
		DrawText(text, pos.x, pos.y, 20, BLACK);
	}

	
	// {
	// 	size offset = offset_of(GameLevel, tower);
	// 	Entity *tower = (Entity *) (((u8 *) Level) + offset);
	// 	const cstr *text = TextFormat("tower health: %.f", tower->health);
	// 	i32 size = MeasureText(text, 10);
	// 	V2 pos = Vec2(Data->canvas_size.x * 0.5f - size * 0.5f, 22);
	// 	DrawText(text, pos.x, pos.y, 10, RED);
	// }
	// Draw editor
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
	level->cake = create_entity((Entity) {
		.type = EntityMainTower,
		.pos = tower_pos,
		.size = tower_size,
		.render.color = PURPLE,
		.health = tower_health,
	});

	// Init Entitys
	size max_turrets = floors * 2;
	size max_projectiles = 400;
	size max_enemys = floors * 10 * 2;
	size max_total = max_turrets + max_projectiles + max_enemys;
	da_init_and_alloc(level->entitys, max_total, sizeof(Entity));
	level->entitys.count = max_total;

	Entity *p = level->entitys.items;
	da_init(level->turrets, max_turrets, p);
	da_init(level->projectiles, max_projectiles, p + max_turrets);
	da_init(level->enemys, max_enemys, p + max_turrets + max_projectiles);


	// Init Wave Manager
	{
		level->wave_manager = (WaveManager) { 0 };
		level->wave_manager.floor_limit = 1;
		level->wave_manager.time_until_next_wave = 5;
		level->wave_manager.packets_max = floors * 10;
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

		push_entity(&level->turrets, create_entity((Entity) {
			.type = EntityTurret,
			.pos = pos,
			.size = Vec2(turret_width, floor_height),
			.render.color = RED,
			.turret.fire_rate = fire_rate,
			.turret.damage = 4,
			.turret.range = 30,
			.health = 100,
			.floor = floor,
		}));
	}

	return (level);
}

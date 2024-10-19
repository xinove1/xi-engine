#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif

GameLevel *create_level(GameData *data, size floors);

global GameData  *Data = NULL;
global GameLevel *Level= NULL;

hot GameConfig init_pre_raylib(void **data)
{
	Data = calloc(1, sizeof(GameData));
	*data = Data;

	*Data = (GameData) {
		.canvas_size = (V2) {640, 360},
		//.canvas_size = (V2) {1280, 720},
		.paused = false,
		.level = NULL,
		.menu_screen = false,
	};

	Data->level = create_level(Data, 5);
	Level = Data->level;

	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Cake Defense",
		.window_flags = FLAG_WINDOW_RESIZABLE,
		.target_fps = 60,
	});
}

hot void init_pos_raylib(void) 
{
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

	if (Level->tower.health <= 0) {
		Data->lost = true;
	}

	update_entity_veffects(&Level->tower);
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

	// TODO  Macro for checking if entity is an empty one, continue if that's the case,
	// and check if its of expected type and give warning if it does not match
	
	// ----------- Update Entitys ----------- 
	
	// ----------- Enemys ----------- 
	{entitys_iterate(Level->enemys) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityEnemy);

		if (entity_died(Level, e)) continue;

		V2 dir = V2DirTo(e->pos, Vec2(Data->canvas_size.x * 0.5f, e->pos.y));
		Entity *target = get_turret(Data, Level->turrets, e->floor, dir.x);
		if (!target) target = &Level->tower; // Attack Main tower instead

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

	{entitys_iterate(Level->spawners) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityEnemySpawner);
		
		e->spawner.rate_count += GetFrameTime();
		if (e->spawner.rate != 0 && e->spawner.rate_count >= e->spawner.rate) {
			e->spawner.rate_count = 0;
			V2 pos = V2Add(e->pos, V2Scale(e->size, 0.5f));
			push_entity(&Level->enemys, create_enemy_(pos, (CreateEnemyParams) {
				.size = Vec2(5, 5),
				.health = 50,
				.color = BLUE,
				.speed = 70,
				.floor = e->floor,
				.melee = false,
				.damage = 1,
				.range = 30,
				.attack_rate = 0.5f
			}));
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
			Entity *target = get_closest_entity(Level->enemys, e->pos);
			if (target) {
				push_entity(&Level->projectiles, create_projectile(e->pos, target->pos, .size = Vec2v(3), .targeting = EntityEnemy, .speed = 200, .damage = 10)); // NOLINT
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

	// // Draw Background outside of map
	// DrawRectangle(0, 0, Data->canvas_size.x, Data->canvas_size.y, BLACK);

	// // Draw Map borders & backgroud/ground of map
	// DrawRectangle(Level->map_offset.x, Level->map_offset.y, Level->map_sz.x * TILE, Level->map_sz.y * TILE, WHITE);
	//

	render_entity(&Level->tower);
	{entitys_iterate(Level->entitys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;
		render_entity(e);
	}}

	{
		cstr *text = TextFormat("Tower health: %.f/%.f", Level->tower.health, Level->tower.health_max);
		i32 size = MeasureText(text, 10);
		V2 pos = Vec2(Data->canvas_size.x * 0.5f - size * 0.5f, 12);
		DrawText(text, pos.x, pos.y, 10, RED);
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
	f32 floor_height = 20;
	f32 floor_padding = 5;
	f32 turret_width = 25;
	f32 tower_width = 50;
	f32 tower_health = 400;
	f32 ground_height = 50;

	// TODO  

	V2 canvas = data->canvas_size;
	V2 canvas_middle = Vec2(canvas.x * 0.5f, canvas.y * 0.5f);
	V2 tower_size = Vec2(tower_width, (floor_height * floors) + (floor_padding * floors));
	V2 tower_pos = Vec2(canvas_middle.x - (tower_size.x * 0.5f), canvas.y - (tower_size.y + ground_height));
	level->tower = create_entity((Entity) {
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
	size max_spawners = floors * 2;
	size max_total = max_turrets + max_projectiles + max_enemys + max_spawners;
	da_init_and_alloc(level->entitys, max_total, sizeof(Entity));
	level->entitys.count = max_total;

	Entity *p = level->entitys.items;
	da_init(level->turrets, max_turrets, p);
	da_init(level->projectiles, max_projectiles, p + max_turrets);
	da_init(level->enemys, max_enemys, p + max_turrets + max_projectiles);
	da_init(level->spawners, max_spawners, p + max_turrets + max_projectiles + max_enemys);

	f32 spawn_rate = 1;

	// Spawners
	for (i32 i = 0; i < max_spawners; i++) {
		if (i == 0) spawn_rate = 1;
		else spawn_rate = 0;

		V2 pos = {0, canvas.y - ground_height};
		i32 floor = 0;
		// Left side
		if (i < max_spawners * 0.5f) {
			floor = i + 1;
			pos.x = 0;
		}
		else { // Right side
			floor = i - max_spawners/2.f +1;
			pos.x = canvas.x - turret_width;
		}
		pos.y -= (floor_padding + floor_height) * floor;
		
		push_entity(&level->spawners, create_entity((Entity) {
			.type = EntityEnemySpawner,
			.pos = pos,
			.render.color = GRAY,
			.size = Vec2(turret_width, floor_height),
			.spawner.rate = spawn_rate,
			.floor = floor,
		}));
	}

	// Turrets
	for (i32 i = 0; i < max_turrets; i++) {
		f32 fire_rate = 0;
		if (i == 0) fire_rate = 1;
		V2 pos = {0, canvas.y - ground_height};
		i32 floor = 0;
		// Left side
		if (i < max_turrets * 0.5f) {
			floor = i + 1;
			pos.x = tower_pos.x -  turret_width;
		}
		else { // Right side
			floor = i - max_turrets/2.f +1;
			pos.x = tower_pos.x + tower_width;
		}
		pos.y -= (floor_padding + floor_height) * floor;

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

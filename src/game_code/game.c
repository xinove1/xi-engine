#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif


global GameData  *Data = NULL;
global GameLevel *Level= NULL;

hot GameConfig init_pre_raylib(void **data)
{
	Data = calloc(1, sizeof(GameData));
	*data = Data;

	*Data = (GameData) {
		.canvas_size = (V2) {640, 360},
		//.canvas_size = (V2) {800, 600},
		.paused = false,
		.level = NULL,
		.menu_screen = false,
	};

	// Teste Level
	{
		Data->level = calloc(1, sizeof(GameLevel));
		Level = Data->level;

		da_init(Level->enemys, 10, sizeof(Entity));
		da_init(Level->turrets, 10, sizeof(Entity));
		da_init(Level->projectiles, 10, sizeof(Entity));
		da_init(Level->spawners, 10, sizeof(Entity));
		da_init(Level->effects, 30, sizeof(Effect));

		create_entity(&Level->spawners, (Entity) {
			.type = EntityEnemySpawner,
			.pos = Vec2(Data->canvas_size.x - 5, Data->canvas_size.y * 0.5f),
			.color = GRAY,
			.size = Vec2v(5),
			.spawner.rate = 2,
		});

		create_entity(&Level->turrets, (Entity) {
			.type = EntityTurret,
			.pos = Vec2(Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.5f),
			.size = Vec2(10, 15),
			.color = RED,
			.turret.fire_rate = 1.0f,
			.turret.damage = 4,
			.turret.range = 30,
			.health = 100,
		});

		create_entity(&Level->turrets, (Entity) {
			.type = EntityTurret,
			.pos = Vec2(Data->canvas_size.x * 0.8f, Data->canvas_size.y * 0.5f),
			.size = Vec2(10, 5),
			.color = RED,
			.turret.fire_rate = 1.6f,
			.turret.damage = 4,
			.turret.range = 30,
			.health = 10,
		});
	}


	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Tower Defense",
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
}


hot b32 update(void)
{
	assert(Data && Level);
	if (Data->menu_screen) {
		return (false);
	}

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

		Entity *target = get_closest_entity(Level->turrets, e->pos);
		if (!target) continue;

		if (IsRecInRange(RecV2(target->pos, V2Add(target->size, e->size)), e->pos, e->enemy.range)) {
			e->enemy.attack_rate_count += GetFrameTime();
			if (e->enemy.attack_rate_count >= e->enemy.attack_rate) {
				e->enemy.attack_rate_count = 0;
				damage_entity(Level, target, e->enemy.damage);
			}
		} else { // Move Towards Turret
			V2 dir = V2Normalize(V2Subtract(target->pos, e->pos));
			e->pos = V2Add(e->pos, V2Scale(dir, e->enemy.speed * GetFrameTime()));
		}
	}}

	{entitys_iterate(Level->spawners) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityEnemySpawner);
		
		e->spawner.rate_count += GetFrameTime();
		if (e->spawner.rate_count >= e->spawner.rate) {
			e->spawner.rate_count = 0;
			create_entity(&Level->enemys, (Entity) {
				.type = EntityEnemy,
				.size = Vec2(5, 5),
				.pos = e->pos,
				.color = BLUE,
				.health = 50,
				.enemy.speed = 70,
				.enemy.melee = true,
				.enemy.damage = 1,
				.enemy.range = 5,
			});
		}
	}}

	// ----------- Turrets ----------- 
	{entitys_iterate(Level->turrets) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityTurret);

		if (entity_died(Level, e)) continue;

		e->turret.fire_rate_count += GetFrameTime();
		if (e->turret.fire_rate_count >= e->turret.fire_rate) {
			e->turret.fire_rate_count = 0;
			Entity *target = get_closest_entity(Level->enemys, e->pos);
			if (target) {
				create_projectile(&Level->projectiles, e->pos, target->pos, .size = Vec2v(3), .targeting = EntityEnemy, .speed = 200, .damage = 10); // NOLINT
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

	apply_effects(Level->effects);

	update_editor();
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

	{entitys_iterate(Level->enemys) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue ;
		render_entity(e);
	}}

	{entitys_iterate(Level->turrets) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;
		render_entity(e);
	}}

	{entitys_iterate(Level->projectiles) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue;
		render_entity(e);
	}}
	
	#ifdef BUILD_DEBUG

	{entitys_iterate(Level->spawners) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) continue ;
		render_entity(e);
	}}

	#endif


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

#include "game.h"

void update_turret(GameLevel *l, Turret *turret) 
{
	if (turret->health <= 0) {
		turret->type = EntityEmpty;
		return ;
	}

	turret->fire_rate_count += GetFrameTime();
	if (turret->fire_rate && turret->fire_rate_count >= turret->fire_rate) {
		turret->fire_rate_count = 0;
		Enemy *target = turret_get_target(l->enemys, *turret, 1); // TODO  Add floor range to turret
		if (target) {
			V2 pe = V2Add(turret->pos, V2Scale(turret->size, 0.5));
			V2 pt = V2Add(target->pos, V2Scale(target->size, 0.5));
			spawn_projectile(pe, pt, .size = Vec2(2, 2), .targeting = EntityEnemy, .speed = 400, .damage = 10); 
		}
	}
}

void update_enemy(GameLevel *l, Enemy *enemy)
{
	if (enemy->health <= 0 ) {
		enemy->type = EntityEmpty;
		return;
	}

	V2 dir = V2DirTo(enemy->pos, Vec2(Data->canvas_size.x * 0.5f, enemy->pos.y));
	GenericEntity *target = (GenericEntity *)enemy_get_turret(l->turrets, enemy->floor, dir.x);
	if (!target) target = &l->cake; // Attack Main tower instead

	if (entity_in_range((GenericEntity *)enemy, (GenericEntity *)target, enemy->range)) {
		enemy->attack_rate_count += GetFrameTime();
		if (enemy->attack_rate_count >= enemy->attack_rate) {
			enemy->attack_rate_count = 0;
			if (enemy->melee == true) {
				damage_entity(l, target, enemy->damage);
			} else {
				V2 pe = V2Add(enemy->pos, V2Scale(enemy->size, 0.5));
				V2 pt = V2Add(target->pos, V2Scale(target->size, 0.5));
				if (target == &l->cake) { pt = Vec2(target->pos.x, pe.y); }
				spawn_projectile(pe, pt, .size = Vec2(3, 2), .targeting = target->type, .speed = 200, .damage = enemy->damage); 
			}
		}
	} else { // Move Towards Turret
		enemy->pos = V2Add(enemy->pos, V2Scale(dir, enemy->speed * GetFrameTime()));
	}
}

void update_projectile(GameLevel *l, Projectile *projectile)
{
	if (projectile->health <= 0) {
		// TODO  Move this to after hit a enemy an splatter the particles in ther direction of the collision
		for (i32 i = 0; i < 10; i++) {
			create_particle( 
				.dir = Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1)),
				.velocity = GetRandf32(50, 200),
				.pos = V2Add(projectile->pos, Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1))),
				.size = Vec2v(1),
				.duration = GetRandf32(0.1f, 0.4f),
				.color_initial = ColA(255, 0, 0, 255),
				.color_end = ColA(255, 0, 0, 0),
		);
		}
		projectile->type = EntityEmpty;
	}

	projectile->pos = V2Add(projectile->pos, V2Scale(projectile->dir, projectile->speed * GetFrameTime()));
	if (!CheckCollisionRecs(RecV2(V2Zero(), Data->canvas_size), RecV2(projectile->pos, projectile->size))) {
		projectile->type = EntityEmpty;
		return ;
	}

	Rect rec = RecV2(projectile->pos, projectile->size);
	switch (projectile->targeting) {
		case EntityTurret: {
			{da_iterate(l->turrets, TurretDa){
				Turret *turret = iterate_get();
				iterate_check_entity(turret, EntityTurret);
				if (CheckCollisionRecs(rec, RecV2(turret->pos, turret->size))) {
					damage_entity(l, (GenericEntity *)turret, projectile->damage);
					projectile->health -= turret->health;
					break;
				}
			}}
		} break;
		case EntityEnemy: {
			{da_iterate(l->enemys, EnemyDa){
				Enemy *enemy = iterate_get();
				iterate_check_entity(enemy, EntityEnemy);
				if (CheckCollisionRecs(rec, RecV2(enemy->pos, enemy->size))) {
					damage_entity(l, (GenericEntity *)enemy, projectile->damage);
					projectile->health -= enemy->health;
					break;
				}
			}}
		} break;
		case EntityCake: {
			if (CheckCollisionRecs(rec, RecV2(l->cake.pos, l->cake.size))) {
				damage_entity(l, (GenericEntity *) &l->cake, projectile->damage);
				projectile->health -= l->cake.health;
			}
		} break;
		default: {
			TraceLog(LOG_WARNING, "projectile update: targeting type not implemented %s\n", EntityTypeNames[projectile->targeting]);
		} break ;
	}
}

void damage_entity(GameLevel *rt, GenericEntity *entity, f32 damage)
{
	assert(entity && rt);
	
	switch (entity->type) {
		case EntityEmpty: {
			TraceLog(LOG_WARNING, "damage_entity: can't damage %s", EntityTypeNames[entity->type]);
		} break;

		case EntityTurret:
		case EntityProjectile:
		case EntityCake:
		case EntityEnemy: {
			apply_flash_effect(entity, BLACK, 0.25f);
			apply_shake_effect(entity, 0.25f);
			entity->health -= damage;
		} break ;

		default: {
			TraceLog(LOG_WARNING, "damage_entity: damaging %s not implemented yet.", EntityTypeNames[entity->type]);
		} break;
	}
}

void apply_func_entitys(GameLevel *l, void (*func)(GenericEntity *entity)) 
{
	func(&l->cake);
	{da_iterate(l->turrets, TurretDa) {
		GenericEntity *e = (GenericEntity *) iterate_get();
		if (e->type == EntityEmpty) continue;
		func(e);
	}}
	{da_iterate(l->enemys, EnemyDa) {
		GenericEntity *e = (GenericEntity *) iterate_get();
		if (e->type == EntityEmpty) continue;
		func(e);
	}}
	{da_iterate(l->projectiles, ProjectileDa) {
		GenericEntity *e = (GenericEntity *) iterate_get();
		if (e->type == EntityEmpty) continue;
		func(e);
	}}
}

Enemy *turret_get_target(EnemyDa enemys, Turret turret, i32 floor_variance)
{
	Enemy *r = NULL;

	V2 from_side = V2DirTo(turret.pos, Vec2(Data->canvas_size.x * 0.5f, turret.pos.y));;
	f32 closest_dist = -1;
	{da_iterate(enemys, EnemyDa) {
		Enemy *e = iterate_get();
		if (e->type == EntityEmpty) continue ;

		if (e->floor > turret.floor + floor_variance || e->floor < turret.floor - floor_variance) 
			continue;

		V2 side = V2DirTo(e->pos, Vec2(Data->canvas_size.x * 0.5f, e->pos.y));
		if (side.x != from_side.x) continue;
		f32 dist = fabs(V2Distance(turret.pos, e->pos));
		if (dist < closest_dist || closest_dist == -1) {
			closest_dist = dist;
			r = e;
		}
	}}

	return (r);
}

b32 entity_in_range(GenericEntity *from, GenericEntity *to, f32 range) 
{
	//Rect from_rec = RecV2(V2Subtract(from->pos, Vec2v(range)), V2Add(from->size, Vec2v(range)));
	Rect from_rec = RecGrow(RecV2(from->pos, from->size), range);
	Rect to_rec = RecV2(to->pos, to->size);
	return (CheckCollisionRecs(from_rec, to_rec));
}

Turret create_turret(Turret turret)
{
	if (V2Compare(turret.render.size, V2Zero())) {
		turret.render.size = turret.size;
	}
	if (turret.health_max == 0) {
		turret.health_max = turret.health;
	}
	return (turret);
}

bool spawn_turret(GameLevel *level, Turret turret) 
{
	{da_iterate(level->turrets, TurretDa) {
		Turret *e = iterate_get();
		if (e->type == EntityEmpty) {
			*e = turret;
			return (true);
		}
	}}
	if (level->turrets.count >= level->turrets.capacity) {
		TraceLog(LOG_WARNING, "spawn_turret: level->turrets is full.");
	} else {
		level->turrets.items[level->turrets.count] = turret;
		level->turrets.count++;
		return (true);
	}
	return (false);
}

Projectile *spawn_projectile_ex(V2 from, V2 to, CreateProjectileParams params) 
{
	V2 dir = V2Normalize(V2Subtract(to, from));

	Projectile e =  {
		.type = EntityProjectile,
		.pos = from,
		.size = params.size,
		.health = params.health,
		.health_max = params.health,
		.render.tint = params.color,
		.render.size = params.size,
		.targeting = params.targeting,
		.dir = dir,
		.speed = params.speed,
		.damage = params.damage,
	};

	ProjectileDa *da = &Data->level->projectiles;
	Projectile *p = NULL;
	{da_iterate(*da, ProjectileDa) {
		p = iterate_get();
		if (p->type == EntityEmpty) {
			*p = e;
			return (p);
		}
	}}

	if (da->count < da->capacity) {
		p = &da->items[da->count];
		*p = e;
		da->count++;
	} else {
		TraceLog(LOG_WARNING, "spawn_projectile_ex: ProjectileDa is full.");
	}
	return (p);
}

Enemy create_enemy_ex(V2 pos, CreateEnemyParams params) 
{
	Enemy enemy = (Enemy) {
		.type = EntityEnemy,
		.pos = pos,
		.size = params.size,
		.render = params.render,
		.health = params.health,
		.floor = params.floor,
		.speed = params.speed,
		.melee = params.melee,
		.damage = params.damage,
		.range = params.range,
		.attack_rate = params.attack_rate,
	};

	if (V2Compare(enemy.render.size, V2Zero())) {
		enemy.render.size = enemy.size;
	}
	if (enemy.health_max == 0) {
		enemy.health_max = enemy.health;
	}

	return (enemy);
}

Enemy create_enemy_prefab(EnemyPrefabs type) 
{
	Enemy enemy = {0};
	Texture2D empty = {0};

	switch (type) {
		case ENEMY_ANT: {
			enemy = create_enemy_ex(Vec2v(0), (CreateEnemyParams) {
				.size = Vec2(32, 32),
				.health = 100,
				.render = CreateSpriteAnimation(Data->assets.sheet_ant, Vec2v(32), .frame_duration = 0.13, .frame_start = 0, .tint = BROWN),
				.speed = 70,
				.floor = 0,
				.melee = true,
				.damage = 2,
				.range = 10,
				.attack_rate = 0.7f
			});
		} break;
		case ENEMY_BEE: {
			enemy = create_enemy_ex(Vec2v(0), (CreateEnemyParams) {
				.size = Vec2(32, 32),
				.health = 80,
				.render = CreateSpriteAnimation(empty, Vec2v(32), .frame_duration = 0.13, .frame_start = 0, .tint = YELLOW),
				.speed = 120,
				.floor = 0,
				.melee = false,
				.damage = 5,
				.range = 100,
				.attack_rate = 1.1f
			});
		} break;
		case ENEMY_NONE:
		case ENEMY_COUNT:
		default: {
			TraceLog(LOG_WARNING, "create_enemy_prefab: type not implemented or invalid: %d \n", type);
		} break;
	}

	return (enemy);
}

bool spawn_enemy(GameLevel *level, Enemy enemy)
{
	{da_iterate(level->enemys, EnemyDa) {
		Enemy *e = iterate_get();
		if (e->type == EntityEmpty) {
			*e = enemy;
			return (true);
		}
	}}
	if (level->enemys.count >= level->enemys.capacity) {
		TraceLog(LOG_WARNING, "add_enemy: level->enemys is full.");
	} else {
		level->enemys.items[level->enemys.count] = enemy;
		level->enemys.count++;
		return (true);
	}
	return (false);
}

Turret *enemy_get_turret(TurretDa turrets, i32 floor, i32 side)
{
	{da_iterate(turrets, TurretDa) {
		Turret *e = iterate_get();
		iterate_check_entity(e, EntityTurret);
		if (e->floor != floor) continue;
		V2 dir = V2DirTo(e->pos, Vec2(Data->canvas_size.x * 0.5f, e->pos.y));
		if (dir.x == side) return (e);
	}}
	return NULL;
}

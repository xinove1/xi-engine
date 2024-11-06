#include "game.h"

void damage_entity(GameLevel *rt, GenericEntity *entity, f32 damage)
{
	assert(entity && rt);
	
	switch (entity->type) {
		case EntityEmpty:
		{
			TraceLog(LOG_WARNING, "damage_entity: can't damage %s", EntityTypeNames[entity->type]);
		} break;

		case EntityTurret:
		case EntityProjectile:
		case EntityCake:
		case EntityEnemy:
		{
			// TODO  Apply effect
		//	push_effect(&rt->effects, create_flash_effect(entity, 0.5f, RED, &entity->health, &damage, sizeof(damage)));
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
		.render.color = params.color,
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
		.render.color = params.color,
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

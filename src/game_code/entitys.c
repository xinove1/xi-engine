#include "game.h"

void damage_entity(GameLevel *rt, Entity *entity, f32 damage)
{
	assert(entity && rt);
	
	switch (entity->type) {
		case EntityEmpty:
		case EntityEnemySpawner:
		{
			TraceLog(LOG_WARNING, "damage_entity: can't damage %s", EntityTypeNames[entity->type]);
		} break;

		case EntityTurret:
		case EntityProjectile:
		case EntityMainTower:
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

b32  entity_died(GameLevel *rt, Entity *entity)
{
	assert(entity && rt);
	
	if (entity->health > 0) return (false);
	switch (entity->type) {
		case EntityEmpty:
		case EntityEnemySpawner:
		{
			TraceLog(LOG_WARNING, "entity_died: %s type can't die.", EntityTypeNames[entity->type]);
		} break;

		case EntityTurret:
		case EntityProjectile:
		case EntityEnemy:
		{
			entity->type = EntityEmpty;
			// TODO  Apply death animation
		} break ;

		default: {
			TraceLog(LOG_WARNING, "entity_died: can't damage default");
		} break;
	}
	return (true);
}

Entity *get_closest_entity(EntityDa entitys, V2 from)
{
	Entity *r = NULL;

	f32 closest_dist = -1;
	{entitys_iterate(entitys) {
		Entity *e = iterate_get(entitys);
		if (e->type == EntityEmpty) continue ;

		f32 dist = fabs(V2Distance(from, e->pos));
		if (dist < closest_dist || closest_dist == -1) {
			closest_dist = dist;
			r = e;
		}
	}}

	return (r);
}

Entity *get_closest_entity_range(EntityDa entitys, V2 from, f32 range)
{
	Entity *r = NULL;

	f32 closest_dist = -1;
	{entitys_iterate(entitys) {
		Entity *e = iterate_get(entitys);
		if (e->type == EntityEmpty) continue ;

		f32 dist = fabs(V2Distance(from, e->pos));
		if (dist <= range && (dist < closest_dist || closest_dist == -1)) {
			closest_dist = dist;
			r = e;
		}
	}}

	return (r);
}

b32 EntityInRange(Entity *from, Entity *to, f32 range) 
{
	Rect from_rec = RecV2(V2Subtract(from->pos, Vec2v(range)), V2Add(from->size, Vec2v(range)));
	Rect to_rec = RecV2(to->pos, to->size);
	return (CheckCollisionRecs(from_rec, to_rec));
}

Entity *check_collision(Rect rec, EntityDa entitys) 
{
	{entitys_iterate(entitys) {
		Entity *e = iterate_get(entitys);
		if (e->type == EntityEmpty) continue ;
		if (CheckCollisionRecs(rec, RecV2(e->pos, e->size))) {
			return (e);
		}
	}}

	return (NULL);
}

void push_entity(EntityDa *da, Entity entity) 
{
	assert(da);

	EntityDa _da = *da;
	{entitys_iterate(_da) {
		Entity *e = iterate_get();
		if (e->type == EntityEmpty) {
			*e = entity;
			return ;
		}
	}}

	if (da->count < da->capacity) {
		da->items[da->count] = entity;
		da->count++;
	} else {
		TraceLog(LOG_WARNING, "push_entity: EntityDa is full.");
	}
}

Entity create_entity(Entity entity)
{
	if (V2Compare(entity.render.size, V2Zero())) {
		//TraceLog(LOG_INFO, "create_entity: entity render_size is zeroed, creating one size.");
		entity.render.size = entity.size;
	}
	if (entity.health_max == 0) {
		entity.health_max = entity.health;
	}
	return (entity);
}

Entity create_projectile_(V2 from, V2 to, CreateProjectileParams params) 
{
	V2 dir = V2Normalize(V2Subtract(to, from));

	Entity e = create_entity((Entity) {
		.type = EntityProjectile,
		.pos = from,
		.size = params.size,
		.health = params.health,
		.render.color = params.color,
		.bullet.targeting = params.targeting,
		.bullet.dir = dir,
		.bullet.speed = params.speed,
		.bullet.damage = params.damage,
	});

	return (e);
}

Entity create_enemy_(V2 pos, CreateEnemyParams params) 
{
	Entity e = create_entity((Entity) {
		.type = EntityEnemy,
		.pos = pos,
		.size = params.size,
		.render.color = params.color,
		.health = params.health,
		.floor = params.floor,
		.enemy.speed = params.speed,
		.enemy.melee = params.melee,
		.enemy.damage = params.damage,
		.enemy.range = params.range,
		.enemy.attack_rate = params.attack_rate,
	});

	return (e);
}

Entity *get_turret(GameData *data, EntityDa towers, i32 floor, i32 side)
{
	{entitys_iterate(towers) {
		Entity *e = iterate_get();
		iterate_check_entity(e, EntityTurret);
		if (e->floor != floor) continue;
		V2 dir = V2DirTo(e->pos, Vec2(data->canvas_size.x * 0.5f, e->pos.y));
		if (dir.x == side) return (e);
	}}

	return NULL;
}

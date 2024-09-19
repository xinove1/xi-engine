#include "game.h"

void render_entity(Entity *entity)
{
	if (entity->type == EntityEmpty) return ;

	Rect rec = RecV2(V2Add(entity->pos, entity->render_pos_offset), entity->render_size);
	DrawRectangleRec(rec, entity->color);
}

void create_entity(EntityDa *da, Entity entity) 
{
	assert(da);

	if (V2Compare(entity.render_size, V2Zero())) {
		//TraceLog(LOG_INFO, "create_entity: entity render_size is zeroed, creating one size.");
		entity.render_size = entity.size;
	}

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
		TraceLog(LOG_WARNING, "create_entity: EntityDa is full.");
	}
}

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
		case EntityEnemy:
		{
			push_effect(&rt->effects, create_flash_effect(entity, 0.5f, RED, &entity->health, &damage, sizeof(damage)));
		//	entity->health -= damage;
			// TODO  Apply effect
		} break ;

		default: {
			TraceLog(LOG_WARNING, "damage_entity: can't damage default");
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

void create_projectile_(EntityDa *da, V2 from, V2 to, CreateProjectileParams params) 
{
	V2 dir = V2Normalize(V2Subtract(to, from));
	create_entity(da, (Entity) {
		.type = EntityProjectile,
		.pos = from,
		.size = params.size,
		.health = params.health,
		.color = params.color,
		.bullet.targeting = params.targeting,
		.bullet.dir = dir,
		.bullet.speed = params.speed,
		.bullet.damage = params.damage,
	});
}

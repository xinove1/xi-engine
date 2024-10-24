#ifndef ENTITYS_H_
# define ENTITYS_H_
# include "raylib.h"
# include "core.h"
# include "render.h"

# define iterate_check_entity(e, expected_type)        \
	assert(e);                             \
	if (e->type == EntityEmpty) {continue;} \
	if (e->type != expected_type) { TraceLog(LOG_WARNING, "iterate_check_entity: expected entity of type %s but got type %s. continuing", EntityTypeNames[expected_type], EntityTypeNames[e->type]); continue;}

introspect() typedef enum {
	EntityEmpty,
	EntityTurret,
	EntityMainTower,
	EntityProjectile,
	EntityEnemy,
	EntityEnemySpawner,
	EntityTypeCount
} EntityType;

typedef struct {
	EntityType type;
	RenderData render;
	VEffect veffects[VEffectTypeCount];
	V2 pos;
	V2 size;
	f32 health;
	f32 health_max;
	i32 floor;
	union {
		struct {
			f32 fire_rate;
			f32 fire_rate_count;
			f32 damage;
			f32 range;
		} turret;

		struct {
			EntityType targeting;
			V2 dir;
			f32 speed;
			f32 damage;
		} bullet;

		struct {
			b32 melee;
			f32 speed;
			f32 damage;
			f32 range;
			f32 attack_rate;
			f32 attack_rate_count;
		} enemy;

		struct {
			f32 rate;
			f32 rate_count;
		} spawner;
	};
} Entity;

typedef struct {
	Entity *items;
	size count;
	size capacity;
} EntityDa;

typedef struct {
	Entity **items;
	size count;
	size capacity;
} EntityHolderDa;

typedef struct {
	EntityType targeting;
	V2 size;
	f32 speed;
	f32 damage;
	f32 health;
	Color color;
} CreateProjectileParams;

typedef struct {
	V2 size;
	f32 health;
	Color color;
	i32 floor;
	f32 speed;
	b32 melee;
	f32 damage;
	f32 range;
	f32 attack_rate;
} CreateEnemyParams;

#define create_projectile(from, to, ...)  \
	create_projectile_ex(from, to, (CreateProjectileParams) {.size = (V2) {1,1}, .health = 1, .speed = 10, .color = BLACK, .damage = 1, __VA_ARGS__})

#define create_enemy(pos, ...) \
	create_enemy_ex(pos, (CreateEnemyParams) {__VA_ARGS__})

#endif

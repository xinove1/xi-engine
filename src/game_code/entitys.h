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

// TODO  introspected GenericEntity to have macro to generate header on all other entitys
typedef struct {
	EntityType type;
	RenderData render;
	VEffect veffects[VEffectTypeCount]; // TODO  Rename to render effects
	V2 pos;
	V2 size;
	f32 health;
	f32 health_max;
} GenericEntity;

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
			b32 melee;
			f32 speed;
			f32 damage;
			f32 range;
			f32 attack_rate;
			f32 attack_rate_count;
		} enemy;
	};
} Entity;


typedef struct {
	Entity *items;
	size count;
	size capacity;
} EntityDa;


typedef struct {
	EntityType type;
	RenderData render;
	VEffect veffects[VEffectTypeCount]; // TODO  Rename to render effects
	V2 pos;
	V2 size;
	f32 health;
	f32 health_max;

	// --
	EntityType targeting;
	V2 dir;
	f32 speed;
	f32 damage;
} Projectile;

typedef struct {
	Projectile *items;
	size count;
	size capacity;
} ProjectileDa;

typedef struct {
	V2 point;
	f32 cooldown;
	i32 floor;
} SpawnLocation;

typedef struct {
	Entity enemy;
	f32 cooldown; // cooldown to be aplied to the spawn location
} SpawnPacket;

typedef struct {
	i32 wave;
	i32 floor_limit; // Amount of floors that enemys can spawn in
	f32 time_until_next_wave;
	f32 time_count;
	SpawnLocation *locations; // Length determinted by number of floors on level
	SpawnPacket *packets;
	size packets_amount;
	size packets_max;
	size packet_current;
} WaveManager;

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

#define spawn_projectile(from, to, ...)  \
	spawn_projectile_ex(from, to, (CreateProjectileParams) {.size = (V2) {1,1}, .health = 1, .speed = 10, .color = BLACK, .damage = 1, __VA_ARGS__})

#define create_enemy(pos, ...) \
	create_enemy_ex(pos, (CreateEnemyParams) {__VA_ARGS__})

#endif

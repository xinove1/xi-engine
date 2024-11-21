#ifndef RENDER_H_
# define  RENDER_H_
# include "core.h"
# include "raylib.h"
# include "sprite.h"

introspect() typedef enum {
	VEffectEmpty, VEffectFlashColor, VEffectShake, VEffectTypeCount
} VEffectType;

typedef struct {
	VEffectType type;
	bool finished;
	f32 duration;
	f32 duration_count;
	union {
		// Flash
		struct {
			Color color_original;
			Color color_target;
		};
	};
} VEffect;

introspect() typedef enum {
	ParticleEmpty, ParticleAlive, ParticleTypeCount
} ParticleType;

typedef struct {
	ParticleType type;
	Color color_initial;
	Color color_end;
	V2 pos;
	V2 size;
	V2 dir;
	f32 velocity;
	f32 duration;
	f32 duration_count;
} Particle;

typedef  struct {
	Color color_initial;
	Color color_end;
	V2 pos;
	V2 size;
	V2 dir;
	f32 velocity;
	f32 duration;
} CreateParticleParams;

// TODO  Add paralex bacground effect
// TODO  also better name
introspect() typedef enum {
	EnvSpriteEmpty,
	EnvSpriteStatic,
	EnvSpriteMoving,
	EnvSpriteCount,
} EnvSpriteType; 

typedef struct {
	EnvSpriteType type;
	Sprite sprite;
	V2 pos;
	V2 dir;
	f32 speed;
} EnvSprite;

#define create_particle(...) \
	create_particle_ex((CreateParticleParams) {__VA_ARGS__})

#endif 

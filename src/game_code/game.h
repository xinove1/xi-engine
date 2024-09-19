#ifndef GAME_H_
# define GAME_H_

# include "../main.h"
# include "raylib.h"
# include "raymath.h"
# include "raymath_short.h"
# include "core.h"
# include "xui.h"
# include "mui.h"
# include "microui.h"
# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "entitys.h"
# include "effects.h"

#define da_init(da, limit, items_size)  \
	do {                            \
		(da).count = 0;        \
		(da).capacity = limit; \
		(da).items = calloc(limit, items_size); \
	} while (0); \

//#define da_simple_append(da, item) (da)->items[(da)->count++] = (item)

#define da_iterate(da, da_type) da_type __da = da; for (size __i = 0; __i < __da.count; __i++) 
#define entitys_iterate(da) da_iterate(da, EntityDa)
#define effects_iterate(da) da_iterate(da, EffectDa)
#define iterate_get(da) &__da.items[__i]

typedef struct {
	byte *items;
	size count;
	size capacity;
} Da;

typedef struct {
	char *name;
	EntityDa turrets;
	EntityDa enemys;
	EntityDa projectiles;
	EntityDa spawners;
	EffectDa effects;
} GameLevel;

typedef struct
{
	mu_Context *mu;
	Entity *selected;
	Entity *hovered;
} GameEditor;

typedef struct {
	V2 canvas_size;
	b32 paused;
	b32 menu_screen;
	UiContainer menu;
	GameEditor editor;
	GameLevel *level;
} GameData;

// Entitys
void create_entity(EntityDa *da, Entity entity);
void create_projectile_(EntityDa *da, V2 from, V2 to, CreateProjectileParams params);
void damage_entity(GameLevel *rt, Entity *entity, f32 damage);
b32  entity_died(GameLevel *rt, Entity *entity);
Entity *get_closest_entity(EntityDa entitys, V2 from);
Entity *get_closest_entity_range(EntityDa entitys, V2 from, f32 range);
Entity *check_collision(Rect rec, EntityDa entitys) ;
void render_entity(Entity *entity);

// Effects
void apply_effects(EffectDa da);
Effect create_flash_effect(Entity *target, f32 duration, Color color, void *data_offset, void *data, size data_size);
void push_effect(EffectDa *da, Effect effect);

// Prototype for not hot reloadable version
GameFunctions game_init_functions();

// Utils
V2 ExpDecayV2(V2 a, V2 b, f32 decay);
b32 IsRecInRange(Rect rec, V2 from, f32 range);
void draw_grid_ex(V2 position, V2 grid_size, i32 tile_size, f32 line_thickness, Color color);
void draw_grid(V2 position, V2 grid_size, i32 tile_size);

// Editor stuff
void init_editor(GameData *data);
void update_editor();
void draw_editor();
void pos_reload_editor(GameData *data);

// exemple
// void _Testfunc(byte *str, GameData data) 
// {
// 	printf("str: %s, data- quit: %d, music_vol: %f\n", str, data.quit, data.music_vol);
// }
// #define Testfunc(str, ...) _Testfunc((str), (GameData){.quit = false, __VA_ARGS__})

#endif // GAME_H_

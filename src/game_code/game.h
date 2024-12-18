#ifndef GAME_H_
# define GAME_H_

# include "raylib.h"
# include "raymath_short.h"
# include "../main.h"
# include "core.h"
# include "xui.h"
# include "sprite.h"
# include "mui.h"
# include <assert.h>
# include <stdio.h>
# include "render.h"
# include "entitys.h"
# include "meta_generated.h"

// TODO  Better name needed for this macros
#define da_init_and_alloc(da, limit, items_size)   \
	do {                                        \
		(da).count = 0;                      \
		(da).capacity = limit;                \
		(da).items = calloc(limit, items_size);\
	} while (0);                                    \

#define da_init(da, limit, start)    \
	do {                          \
		(da).count = 0;        \
		(da).capacity = limit; \
		(da).items = start;    \
	} while (0);                   \

//#define da_simple_append(da, item) (da)->items[(da)->count++] = (item)

#define da_iterate(da, da_type) da_type __da = da; for (size __i = 0; __i < __da.count; __i++) 
#define entitys_iterate(da) da_iterate(da, EntityDa)
#define effects_iterate(da) da_iterate(da, EffectDa)
#define iterate_get() &__da.items[__i]

#define MAX_GAME_SPEED 4
#define TILE_TURRET_SIZE 32

typedef struct {
	byte *items;
	size count;
	size capacity;
} Da;

typedef struct {
	char *name;
	i32 floors_count;
	Turret *turret_hovered;
	Turret *turret_selected;
	GenericEntity cake;
	WaveManager wave_manager;
	TurretDa turrets;
	EnemyDa enemys;
	ProjectileDa projectiles;
	EnvSprite enviromnent_sprites[50];
} GameLevel;

typedef struct
{
	mu_Context *mu;
	GenericEntity *selected;
	GenericEntity *hovered;
	b32 debug_window;
	b32 no_lose;
	b32 debug_select; // TODO BETTER NAME
	f32 effect_duration;
	Color effect_color;
} GameEditor;

typedef struct {
	mu_Context *mu;
	Sprite paused;
	Sprite speed;
	Sprite next_wave;
	Sprite buy_turret;
} GameUi;

typedef struct {
	Font font;
	Texture sheet_ui;
	Texture sheet_ant;
} GameAssets;

typedef struct {
	b32 paused;
	b32 lost;
	b32 menu_screen;
	i32 game_speed;
	V2 canvas_size;
	UiContainer menu;
	GameEditor editor;
	Particle particles[5000];
	GameLevel *level;
	GameAssets assets;
	GameUi ui;
} GameData;

extern GameData *Data;

void pause_game(void);

// Entitys
void update_turret(GameLevel *l, Turret *turret);
void update_enemy(GameLevel *l, Enemy *enemy);
void update_projectile(GameLevel *l, Projectile *projectile);
void damage_entity(GameLevel *rt, GenericEntity *entity, f32 damage);
void apply_func_entitys(GameLevel *l, void (*func)(GenericEntity *entity));
Projectile *spawn_projectile_ex(V2 from, V2 to, CreateProjectileParams params);
#define spawn_projectile(from, to, ...) spawn_projectile_ex(from, to, (CreateProjectileParams) {.size = (V2) {1,1}, .health = 1, .speed = 10, .color = BLACK, .damage = 1, __VA_ARGS__})
Enemy create_enemy_ex(V2 pos, CreateEnemyParams params);
Enemy create_enemy_prefab(EnemyPrefabs type);
//#define create_enemy(pos, ...) create_enemy_ex(pos, (CreateEnemyParams) {__VA_ARGS__})
bool spawn_enemy(GameLevel *level, Enemy enemy);
b32 entity_in_range(GenericEntity *from, GenericEntity *to, f32 range);
Turret *enemy_get_turret(TurretDa turrets, i32 floor, i32 side);
Turret create_turret(Turret entity);
Turret create_turret_prefab(TurretPrefabs type);
bool spawn_turret(GameLevel *level, Turret turret, i32 floor, V2 pos);
Enemy *turret_get_target(EnemyDa enemys, Turret turret, i32 floor_variance);

// Wave Manager
typedef enum {right_side = 1, left_side = -1} FloorSides;
void update_wave_manager(GameLevel *l);
SpawnLocation *get_spawn_point(GameLevel *l, i32 floor, i32 side);
void start_wave(GameLevel *l);

// Render
void render_entity(GenericEntity *entity);
void update_entity_veffects(GenericEntity *entity);
void update_entity_animations(GenericEntity *entity);
void apply_flash_effect(GenericEntity *entity, Color color, f32 duration);
void apply_shake_effect(GenericEntity *entity, f32 duration);
void render_particle(Particle p);
void create_particle_ex(CreateParticleParams param);
void create_env_sprite(EnvSprite *arr, i32 arr_size, EnvSprite env_sprite);
void render_env_sprites(EnvSprite *arr, i32 arr_size);



// Prototype for not hot reloadable version
GameFunctions game_init_functions();

// Utils
Rect GetCanvasRec();
f32 GetRandf32Ex(f32 min, f32 max, f32 acc);
#define GetRandf32(min, max) GetRandf32Ex(min, max, 1000.0f)
V2 ExpDecayV2(V2 a, V2 b, f32 decay);
Color ExpDecayColor(Color a, Color b, f32 decay);
f32 ExpDecayf32(f32 a, f32 b, f32 decay);
Color lerp_color(Color a, Color b, f32 time);
void draw_grid_ex(V2 position, V2 grid_size, i32 tile_size, f32 line_thickness, Color color);
void draw_grid(V2 position, V2 grid_size, i32 tile_size);
Font LoadFontFromImageSheet(Image image, Vector2 grid_size, int first_char);

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

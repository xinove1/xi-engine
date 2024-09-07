#ifndef GAME_H_
# define GAME_H_

# include "../main.h"
# include "raylib.h"
# include "raymath.h"
# include "raymath_short.h"
# include "core.h"
# include "ui.h"
# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

#define TILE 16
#define MAX_ACTUATORS 5 

typedef enum {EntityEmpty, EntityPlayer, EntityActuator, EntityMovable, EntityMixable, EntityStatic, EntitysCount} EntitysTypes;

typedef struct
{
	EntitysTypes type;
	i32 id; // index on entitys array  NOTE  DON'T CHANGE THIS VALUE MANUALLY, how i would love to set this as const but can't have shit in C
	V2 pos;
	V2 look_dir; 
	Color color; 
} Entity;

typedef struct
{
	char *name;
	i32  *map;
	V2   map_sz;
	V2   map_offset; // Offset used For drawing
	Entity *entitys;
	i32    max_entitys;
	i32    entity_count; // Count of how many entitys are currently on entitys array, also used on create entity
	Entity actuators[MAX_ACTUATORS];
	i32    actuators_count;
} GameLevel;

typedef struct
{
	V2 selected_tile;
	b32 putting_new_entity;
	Entity new_entity;
	b32 dragging;
	Entity *dragged_entity;
	UiContainer panel;
} GameEditor;

typedef struct {
	V2 canvas_size;
	b32 paused;
	GameLevel *current_level;
	GameEditor editor;
	b32 menu_screen;
	UiContainer menu;
} GameData;

// Prototype for not hot reloadable version
GameFunctions game_init_functions();

// Utils
V2 ExpDecayV2(V2 a, V2 b, f32 decay);
void draw_grid_ex(V2 position, V2 grid_size, i32 tile_size, f32 line_thickness, Color color);
void draw_grid(V2 position, V2 grid_size, i32 tile_size);

// Entity & map stuff
b32 move_entity(GameLevel *level, Entity *e, V2 where);
b32 move_entity_swap(GameLevel *level, Entity *e, V2 where);
i32 get_map_pos(GameLevel *level, V2 pos);
void set_map_pos(GameLevel *level, V2 pos, i32 value);
Entity *get_entity(GameLevel *level, i32 entity_id);
Entity *get_map_entity(GameLevel *level, V2 pos);
Entity *get_actuator(GameLevel *level, V2 pos);
Entity *create_entity_empty(GameLevel *level);
Entity *create_entity(GameLevel *level, Entity d);
void delete_entity(GameLevel *level, Entity *e);
Entity *create_actuator(GameLevel *level, Entity d);
GameLevel *create_level(char *name, V2 map_size, V2 canvas_size);
void print_entity(Entity e);
void print_map(GameLevel *level);
void print_level(GameLevel *level);

// Editor stuff
void init_editor(GameData *data);
void update_editor();
void draw_editor();
 

// exemple
// void _Testfunc(byte *str, GameData data) 
// {
// 	printf("str: %s, data- quit: %d, music_vol: %f\n", str, data.quit, data.music_vol);
// }
// #define Testfunc(str, ...) _Testfunc((str), (GameData){.quit = false, __VA_ARGS__})

#endif // GAME_H_

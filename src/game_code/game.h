#ifndef GAME_H_
# define GAME_H_

# include "raylib.h"
# include "raymath.h"
# include "raymath_short.h"
# include "core.h"
# include "ui.h"
# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

// Hot reload being enabled implies debug build
#ifdef HOT_RELOAD
# define DEBUG
#endif

#define TILE 16
#define MAX_ACTUATORS 5 

typedef enum {EntityEmpty, EntityPlayer, EntityActuator, EntityMovable, EntityMixable, EntityStatic, EntitysCount} EntitysTypes;

typedef struct
{
	EntitysTypes	type;
	int	id; // index on entitys array  NOTE  DON'T CHANGE THIS VALUE MANUALLY, how i would love to set this as const but can't have shit in C
	V2	pos;
	Color	color; 
	V2	look_dir; 
} Entity;

typedef struct
{
	char	*name;
	int	*map;
	V2	map_sz;
	V2	map_offset; // Offset used For drawing
	Entity	*entitys;
	int	max_entitys;
	int	entity_count; // Count of how many entitys are currently on entitys array, also used on create entity
	Entity	actuators[MAX_ACTUATORS];
	int	actuators_count;
} GameLevel;

typedef struct
{
	V2	selected_tile;
	bool	putting_new_entity;
	Entity	new_entity;
	bool	dragging;
	Entity	*dragged_entity;
	UiContainer	panel;
}GameEditor;

typedef struct {
	V2	canvas_size;
	bool	paused;
	GameLevel	*current_level;
	GameEditor	editor;
	bool		menu_screen;
	UiContainer	menu;
} GameData;

typedef struct GameFunctions
{
	void	(*init)(GameData *data);
	void	(*update)(void);
	void	(*draw)(void);
	void	(*pre_reload)(void);
	void	(*pos_reload)(GameData *data);
} GameFunctions;

// Prototype for not hot reloadable version
GameFunctions	game_init_functions();

// Utils
V2	ExpDecayV2(V2 a, V2 b, float decay);
void	draw_grid_ex(V2 position, V2 grid_size, int tile_size, float line_thickness, Color color);
void	draw_grid(V2 position, V2 grid_size, int tile_size);

// Entity & map stuff
bool move_entity(GameLevel *level, Entity *e, V2 where);
bool move_entity_swap(GameLevel *level, Entity *e, V2 where);
int get_map_pos(GameLevel *level, V2 pos);
void set_map_pos(GameLevel *level, V2 pos, int value);
Entity *get_entity(GameLevel *level, int entity_id);
Entity *get_map_entity(GameLevel *level, V2 pos);
Entity *get_actuator(GameLevel *level, V2 pos);
Entity *create_entity_empty(GameLevel *level);
Entity *create_entity(GameLevel *level, Entity d);
void	delete_entity(GameLevel *level, Entity *e);
Entity *create_actuator(GameLevel *level, Entity d);
GameLevel *create_level(char *name, V2 map_size, V2 canvas_size);
void print_entity(Entity e);
void print_map(GameLevel *level);
void print_level(GameLevel *level);

// Editor stuff
void	init_editor(GameData *data);
void	update_editor();
void	draw_editor();
 

// exemple
// void _Testfunc(byte *str, GameData data) 
// {
// 	printf("str: %s, data- quit: %d, music_vol: %f\n", str, data.quit, data.music_vol);
// }
// #define Testfunc(str, ...) _Testfunc((str), (GameData){.quit = false, __VA_ARGS__})

#endif // GAME_H_

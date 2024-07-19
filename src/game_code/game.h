#ifndef GAME_H_
# define GAME_H_

# include "raylib.h"
# include "raymath.h"
# include "raymath_short.h"
# include "types.h"

#define TILE 16

typedef enum {EntityPlayer, EntityStatic, EntitysCount} EntitysTypes;

typedef struct
{
	EntitysTypes	type;
	V2	pos;
	Color	color; 
} Entity;

typedef struct
{
	EntitysTypes	type;
	V2	pos;
	Color	color; 
	V2	look_dir; 
} PlayerEntity;

typedef struct
{
	char	*name;
	int	*map;
	V2	map_sz;
	Entity	**entitys;
	int	max_entitys;
	PlayerEntity player;
} GameLevel;

typedef struct {
	V2	canvas_size;
	bool	paused;
	GameLevel	*current_level;
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
 
#endif // GAME_H_

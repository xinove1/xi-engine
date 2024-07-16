#ifndef GAME_H_
# define GAME_H_

# include "raylib.h"
# include "raymath.h"
# include "types.h"

typedef struct {
	V2	canvas_size;
	bool	paused;
	V2	pos;
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

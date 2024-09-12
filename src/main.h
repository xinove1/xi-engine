#ifndef MAIN_H_
# define MAIN_H_
# include "core.h"

// Unused for now
// Should this really be handled by the 'engine' layer?
typedef struct {
	// keybinding config?
	// Resolution if supported
	b32 fullscreen;
	f32 volume_music;
	f32 volume_effects;
} UserConfig;

typedef struct
{
	V2 canvas_size;
	cstr *window_name;
	i32 window_flags;
	i32 target_fps;
} GameConfig;

typedef struct GameFunctions
{
	GameConfig (*init_pre_raylib)(void **data);
	void (*init_pos_raylib)(void);
	b32  (*update)(void);
	void (*draw)(void);
	void (*pre_reload)(void);
	void (*pos_reload)(void *data);
} GameFunctions;

void ChangeGameConfig(GameConfig config);

extern GameFunctions game_init_functions();

#endif

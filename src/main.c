#include "game_code/game.h"
#include <sys/stat.h>
#include <errno.h>
#ifdef HOT_RELOAD
# include <dlfcn.h>
#endif
#ifdef PLATFORM_WEB
# include <emscripten/emscripten.h>
#endif

internal inline void reload_game_lib(GameFunctions *game);
internal inline void *dl_load_func(void *libhandle, char *name);
internal inline i32 get_stat_time(cstr *path);
internal void register_actions();
internal void update_and_draw();

global cstr *LibGamePath = "./game.so";
global i32 LibGameTime = 0;
global b32 QuitGame = false;
global RenderTexture2D ScreenTexture;
global GameConfig Config;
global GameFunctions Game = {0};
global void *Data;

int main()
{
	#ifdef HOT_RELOAD
		reload_game_lib(&Game);
	#else
		Game = game_init_functions();
	#endif

	Config = Game.init_pre_raylib(&Data);

	SetConfigFlags(Config.window_flags);
	InitWindow(Config.canvas_size.x, Config.canvas_size.y, Config.window_name);
	SetWindowState(FLAG_WINDOW_MAXIMIZED); // TODO  Move this to GameConfig or UserConfig or something
	InitAudioDevice();
	SetTargetFPS(Config.target_fps);
	SetExitKey(0);
	
	register_actions();

	ScreenTexture = LoadRenderTexture(Config.canvas_size.x, Config.canvas_size.y);
	//SetTextureFilter(screen.texture, TEXTURE_FILTER_BILINEAR);  
	SetTextureFilter(ScreenTexture.texture, TEXTURE_FILTER_ANISOTROPIC_16X);  

	Game.init_pos_raylib();

	#ifdef PLATFORM_WEB
		emscripten_set_main_loop(update_and_draw, 0, 1);
	#else
		while (!WindowShouldClose() && !QuitGame) {
			update_and_draw();
		}
	#endif

	CloseWindow();
	CloseAudioDevice();
	return (0);
}

internal void update_and_draw()
{
	#ifdef HOT_RELOAD
		// NOTE  this seems pretty hackish but olwell
		i32 lib_game_current_time = get_stat_time(LibGamePath);
		static b32 reload = false;
		static i32 reload_count = 0;
	
		if (IsKeyPressed(KEY_R) || lib_game_current_time > LibGameTime) reload = true;
		if (reload) reload_count++;
		if (reload && reload_count > 5) {
			reload = false;
			reload_count = 0;
			Game.pre_reload();
			reload_game_lib(&Game);
			Game.pos_reload(Data);
		}
	#endif

	PoolActions();

	i32 screen_scale = MIN(GetScreenWidth() / Config.canvas_size.x, GetScreenHeight() / Config.canvas_size.y);
	if (screen_scale <= 0) screen_scale = 1;
	V2 window_size_scaled = V2Scale(Config.canvas_size, screen_scale);
	//printf("screen_scale: %d \n", screen_scale);

	// Update virtual mouse (clamped mouse value behind game screen)
	// V2 mouse = GetMousePosition();
	// V2 mouse_virtual = { 0 };
	// mouse_virtual.x = (mouse.x - (GetScreenWidth() - window_size_scaled.x) * 0.5f) / screen_scale;
	// mouse_virtual.y = (mouse.y - (GetScreenHeight() - window_size_scaled.y) * 0.5f) / screen_scale;
	// mouse_virtual = V2Clamp(mouse_virtual, V2Zero(), Data.window_size);

	// Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
	SetMouseOffset(-(GetScreenWidth() - window_size_scaled.x) * 0.5f, -(GetScreenHeight() - window_size_scaled.y) * 0.5f);
	SetMouseScale(1 / (f32)screen_scale, 1 / (f32)screen_scale);

	QuitGame = Game.update();

	BeginTextureMode(ScreenTexture); {
		ClearBackground(RAYWHITE);
		Game.draw();
	} EndTextureMode();

	BeginDrawing(); {
		ClearBackground(BLACK);
		// Draw render texture to screen, properly scaled
		DrawTexturePro(ScreenTexture.texture,
		 (Rect){0, 0, ScreenTexture.texture.width, -ScreenTexture.texture.height}, // Source
		 (Rect){.x = (i32) ((GetScreenWidth() - window_size_scaled.x) * 0.5f), 
		 .y = (i32) ((GetScreenHeight() - window_size_scaled.y) * 0.5f),
		 .width = window_size_scaled.x, .height = window_size_scaled.y }, // Dest
		 V2Zero(),
		 0.0f,
		 WHITE);
		DrawText(TextFormat("%d", GetFPS()), 30, 30, 30, RED);
	} EndDrawing();
}

internal inline void reload_game_lib(GameFunctions *game) 
{
	#ifdef HOT_RELOAD
	local void *libgame = NULL;
	if (libgame != NULL) {
		dlclose(libgame);
	}

	libgame = dlopen(LibGamePath, RTLD_NOW);
	if (libgame == NULL) {
		TraceLog(LOG_FATAL, "ERROR: %s", dlerror());
	}
	LibGameTime = get_stat_time(LibGamePath);

	TraceLog(LOG_INFO, "(Re)Loaded gamelib");
	game->init_pre_raylib = dl_load_func(libgame, "init_pre_raylib");
	game->init_pos_raylib = dl_load_func(libgame, "init_pos_raylib");
	game->update = dl_load_func(libgame, "update");
	game->draw = dl_load_func(libgame, "draw");
	game->pos_reload = dl_load_func(libgame, "pos_reload");
	game->pre_reload = dl_load_func(libgame, "pre_reload");

	#endif
}

internal inline i32 get_stat_time(cstr *path) 
{
	struct stat statbuf = {0};
	if (stat(path, &statbuf) < 0) {
		// NOTE: if output does not exist it 100% must be rebuilt
		//if (errno == ENOENT) return 1;
		TraceLog(LOG_ERROR, "could not stat %s: %s", path, strerror(errno));
		return (0);
	}
	int path_time = statbuf.st_mtime;
	//printf("path_time: %d \n", path_time);
	return (path_time);
}

internal inline void *dl_load_func(void *libhandle, char *name)
{
	#ifdef HOT_RELOAD
	void *func = dlsym(libhandle, name);
	if (func == NULL) {
		TraceLog(LOG_FATAL, "ERROR: %s", dlerror());
	}
	return (func);
	#else
		return (NULL);
	#endif
}

internal void register_actions()
{
	SetGamePadId(0);

	RegisterActionName(RIGHT, "right");
	RegisterInputKeyAction(RIGHT, KEY_D);
	RegisterInputKeyAction(RIGHT, KEY_RIGHT);
	RegisterGamePadButtonAction(RIGHT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
	RegisterGamePadAxisAction(RIGHT, GAMEPAD_AXIS_LEFT_X, 0.5f);


	RegisterActionName(LEFT, "left");
	RegisterInputKeyAction(LEFT, KEY_A);
	RegisterInputKeyAction(LEFT, KEY_LEFT);
	RegisterGamePadButtonAction(LEFT, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
	RegisterGamePadAxisAction(LEFT, GAMEPAD_AXIS_LEFT_X, -0.5f);


	RegisterActionName(UP, "up");
	RegisterInputKeyAction(UP, KEY_W);
	RegisterInputKeyAction(UP, KEY_UP);
	RegisterGamePadButtonAction(UP, GAMEPAD_BUTTON_LEFT_FACE_UP);
	RegisterGamePadAxisAction(UP, GAMEPAD_AXIS_LEFT_Y, -0.5f);


	RegisterActionName(DOWN, "down");
	RegisterInputKeyAction(DOWN, KEY_S);
	RegisterInputKeyAction(DOWN, KEY_DOWN);
	RegisterGamePadButtonAction(DOWN, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
	RegisterGamePadAxisAction(DOWN, GAMEPAD_AXIS_LEFT_Y, 0.5f);


	RegisterActionName(ACTION_1, "action_1");
	RegisterInputKeyAction(ACTION_1, KEY_J);
	RegisterInputKeyAction(ACTION_1, KEY_X);
	RegisterGamePadButtonAction(ACTION_1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
	RegisterGamePadAxisAction(ACTION_1, GAMEPAD_AXIS_RIGHT_TRIGGER, 0.7f);


	RegisterActionName(ACTION_2, "action_2");
	RegisterInputKeyAction(ACTION_2, KEY_K);
	RegisterInputKeyAction(ACTION_2, KEY_Z);
	RegisterGamePadButtonAction(ACTION_2, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);


	RegisterActionName(ACTION_3, "action_3");
	RegisterInputKeyAction(ACTION_3, KEY_SPACE);
	RegisterGamePadButtonAction(ACTION_3, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);


	RegisterActionName(OPEN_MENU, "open_menu");
	RegisterInputKeyAction(OPEN_MENU, KEY_ESCAPE);
	RegisterInputKeyAction(OPEN_MENU, KEY_E);
	RegisterGamePadButtonAction(OPEN_MENU, GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

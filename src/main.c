#include "game_code/game.h"
#include "types.h"
#include "input.h"
#ifdef HOT_RELOAD
# include "dlfcn.h"
#endif

static inline void	reload_game_lib(GameFunctions *game);
static inline void	*dl_load_func(void *libhandle, char *name);

static bool	QuitGame = false;

int	main()
{
	RenderTexture2D	screen;
	V2	window_size = {640, 360};
	GameData	data = {
		.canvas_size = window_size,
	};

	GameFunctions	game = {0};

	#ifdef HOT_RELOAD
		reload_game_lib(&game);
	#else
		game = game_init_functions();
	#endif

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(window_size.x, window_size.y, "Xi engine");
	SetWindowState(FLAG_WINDOW_MAXIMIZED);
	InitAudioDevice();
	SetTargetFPS(60);
	SetExitKey(0);

	screen = LoadRenderTexture(window_size.x, window_size.y);
	//SetTextureFilter(screen.texture, TEXTURE_FILTER_BILINEAR);  
	SetTextureFilter(screen.texture, TEXTURE_FILTER_ANISOTROPIC_16X);  

	game.init(&data);

	while (!WindowShouldClose() && !QuitGame) {
		#ifdef HOT_RELOAD
		if (IsKeyPressed(KEY_R)) {
			game.pre_reload();
			reload_game_lib(&game);
			game.pos_reload(&data);
		}
		#endif

		float screen_scale = MIN((float)GetScreenWidth()/window_size.x, (float)GetScreenHeight() / window_size.y);
		// Update virtual mouse (clamped mouse value behind game screen)
		Vector2 mouse = GetMousePosition();
		Vector2 virtualMouse = { 0 };
		virtualMouse.x = (mouse.x - (GetScreenWidth() - (window_size.x *screen_scale))*0.5f)/screen_scale;
		virtualMouse.y = (mouse.y - (GetScreenHeight() - (window_size.y *screen_scale))*0.5f)/screen_scale;
		virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)window_size.x, (float)window_size.y});

		//Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
		SetMouseOffset(-(GetScreenWidth() - (window_size.x*screen_scale))*0.5f, -(GetScreenHeight() - (window_size.y*screen_scale))*0.5f);
		SetMouseScale(1 / screen_scale, 1 / screen_scale);

		game.update();

		BeginTextureMode(screen);
		{
			ClearBackground(RAYWHITE);
			game.draw();
		}
		EndTextureMode();

		BeginDrawing();
		{
			ClearBackground(BLACK);
			// Draw render texture to screen, properly scaled
			DrawTexturePro(screen.texture,
		  (Rect){0.0f, 0.0f, (float) screen.texture.width, (float) -screen.texture.height},
		  (Rect){(GetScreenWidth() - ((float) window_size.x*screen_scale)) * 0.5f, (GetScreenHeight() - ((float) window_size.y*screen_scale)) * 0.5f,
		  (float)window_size.x * screen_scale, (float)window_size.y * screen_scale },
		  (Vector2){ 0, 0 },
		  0.0f,
			WHITE);
			DrawText(TextFormat("%d", GetFPS()), 30, 30, 30, RED);
		}
		EndDrawing();
	}
	CloseWindow();
	CloseAudioDevice();
	return (0);
}

static inline void	reload_game_lib(GameFunctions *game) 
{
	#ifdef HOT_RELOAD

	static void	*libgame = NULL;
	if (libgame != NULL) {
		dlclose(libgame);
	}

	libgame = dlopen("./game.so", RTLD_NOW);
	if (libgame == NULL) {
		TraceLog(LOG_FATAL, "ERROR: %s", dlerror());
	}
	TraceLog(LOG_INFO, "(Re)Loaded gamelib");
	game->init = dl_load_func(libgame, "init");
	game->update = dl_load_func(libgame, "update");
	game->draw = dl_load_func(libgame, "draw");
	game->pos_reload = dl_load_func(libgame, "pos_reload");
	game->pre_reload = dl_load_func(libgame, "pre_reload");

	#endif
}

static inline void	*dl_load_func(void *libhandle, char *name)
{
	#ifdef HOT_RELOAD
	void	*func = dlsym(libhandle, name);
	if (func == NULL) {
		TraceLog(LOG_FATAL, "ERROR: %s", dlerror());
	}
	return (func);
	#else
		return (NULL);
	#endif
}

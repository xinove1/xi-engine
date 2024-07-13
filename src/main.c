#include "game_code/game.h"
#include "modules/types.h"
#include "modules/input.h"

static void	register_actions();

static bool	QuitGame = false;

int	main()
{
	RenderTexture2D	screen;
	V2	window_size = {640, 360};
	GameData	data = {
		.canvas_size = window_size,
	};

	GameFunctions	game = {0};

	#ifdef HOTRELOAD

	#else
		game = game_init_functions();
	#endif

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(window_size.x, window_size.y, "Xi engine");
	SetWindowState(FLAG_WINDOW_MAXIMIZED);
	InitAudioDevice();
	SetTargetFPS(60);
	SetExitKey(0);

	register_actions();

	screen = LoadRenderTexture(window_size.x, window_size.y);
	//SetTextureFilter(screen.texture, TEXTURE_FILTER_BILINEAR);  
	SetTextureFilter(screen.texture, TEXTURE_FILTER_ANISOTROPIC_16X);  

	game.init(&data);

	while (!WindowShouldClose() && !QuitGame) {
		PoolActions();

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

static void	register_actions()
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

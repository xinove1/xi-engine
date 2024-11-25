#include "game.h"
#include "collision.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define hot
#else
# define hot static
#endif

void init_level(GameLevel *level);
void collide_player(Player *player);
internal b32 update_game(void);
internal b32 update_input(void);
internal b32 update_ui(void);

GameData  *Data = NULL;
global GameLevel *Level= NULL;

// NOTE  for web build
// TODO  Add option on menu to not pause game on leaving canvas focus
void pause_game(void) 
{
	Data->paused = true;
}

hot GameConfig init_pre_raylib(void **data)
{
	Data = calloc(1, sizeof(GameData));
	*data = Data;

	*Data = (GameData) {
		.canvas_size = (V2) {640, 360},
		//.canvas_size = (V2) {1280, 720},
		.particles = {0},
		.paused = false,
		.level = NULL,
		.menu_screen = false,
	};
	Data->ui.mu = calloc(1, sizeof(mu_Context));
	return ((GameConfig) {
		.canvas_size = Data->canvas_size,
		.window_name = "Mini Jam",
		.window_flags = FLAG_WINDOW_RESIZABLE,
		.target_fps = 60,
	});
}

hot void init_pos_raylib(void) 
{
	Image image = LoadImage("assets/monogram-bitmap.png");
	Data->font = LoadFontFromImageSheet(image, Vec2(6, 12), 32);
	UnloadImage(image);
	// Data->assets.sheet_ant = LoadTexture("assets/ant_sheet.png");
	// Data->ui.paused = CreateSpriteSheeted(Data->assets.sheet_ui, Vec2v(16), 0);
	MUiInit(Data->ui.mu, &Data->font, Data->canvas_size);

	Data->menu = XUiCreateContainer((V2) {Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.3f}, 0, (UiConfig) {
			.alignment = UiAlignCentralized,
			.font = (FontConfig) {
				.font = GetFontDefault(),
				.size = 10,
				.spacing = 1,
				.tint = BLACK,
				.tint_hover = RED,
			},
			.draw_container_bounds = true,
			.play_sound = false,
			.draw_selector = false,
			.take_key_input = true,
			.padding_row = 10,
			.padding_collumn = 5,
			.color_background = YELLOW,
			.color_font = RED,
			.color_font_highlight = BLACK,
			.color_borders = BLACK,
	});
	init_editor(Data);
	Level = &Data->level;
	init_level(Level);
}

hot void pre_reload(void)
{

}

hot void pos_reload(void *data)
{
	Data = data;
	Level = &Data->level;
	pos_reload_editor(Data);
}


hot b32 update(void)
{
	assert(Data && Level);

	update_input();
	update_ui();
	update_editor();

	if (Data->menu_screen || Data->lost) {
		return (false);
	}

	if (Data->paused) return (false);

	update_game();

	return (false);
}

internal b32 update_ui(void) 
{
	i32 window_flag = MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_AUTOSIZE | MU_OPT_NORESIZE;
	i32 texture_button = 22;
	mu_Context *ctx = Data->ui.mu; 
	mu_begin(ctx); {

	} mu_end(ctx);

	return (false);
}

internal b32 update_input(void) 
{
	MUiPoolInput(Data->ui.mu);

	Player *player = &Level->player;

	V2 input_dir = {0, 0};
	if (IsActionDown(RIGHT)) {
		input_dir.x += 1;
	}
	if (IsActionDown(LEFT)) {
		input_dir.x -= 1;
	}
	if (IsActionDown(DOWN)) {
		input_dir.y += 1;
	}
	if (IsActionDown(UP)) {
		input_dir.y -= 1;
	}
	if (IsActionDown(ACTION_1) && player->state == PlayerStateWalking) {
		player->state = PlayerStateCharging;
	}

	player->dir = input_dir;
	
	f32 walking_max_vel = 2;
	f32 sliding_max_vel = 20;

	if (player->state == PlayerStateWalking) {
		f32 friction = 0.85f;
		f32 decay = 10;
		f32 speed = 5;
		player->velocity = ExpDecayV2(player->velocity, V2Add(player->velocity, V2Scale(player->dir, speed)), decay);
	///	player->velocity = V2Add(player->velocity, V2Scale(player->dir, speed));
		player->velocity = V2Scale(player->velocity, friction);
		player->velocity = V2ClampValue(player->velocity, -walking_max_vel, walking_max_vel);
		collide_player(player);
		player->pos = V2Add(player->pos, player->velocity);
	} 
	else if (player->state == PlayerStateSliding) {
		f32 friction = 0.95f;
		f32 decay = 15;
		f32 speed = 5;
		player->velocity = ExpDecayV2(player->velocity, V2Add(player->velocity, V2Scale(player->dir, speed)), decay);
		player->velocity = V2Scale(player->velocity, friction);
		player->velocity = V2ClampValue(player->velocity, -sliding_max_vel, sliding_max_vel);
		collide_player(player);
		player->pos = V2Add(player->pos, player->velocity);
		if (V2Length(player->velocity) < walking_max_vel) { 
			player->state = PlayerStateWalking;
		}
	} else if (player->state == PlayerStateCharging) {
		if (IsActionReleased(ACTION_1)) {
			player->velocity = V2Scale(player->dir, sliding_max_vel * 0.8f);
			player->state = PlayerStateSliding;
		}
	}
	 
	#ifdef BUILD_DEBUG
	if (IsKeyPressed(KEY_R)) {
		Level->player.pos = Vec2(Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.5f);
	}
	if (IsKeyPressed(KEY_T)) {
		Level->player.velocity = Vec2v(0);
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		for (i32 j = 0; j < 100; j++){
			create_particle( 
				.dir = Vec2(GetRandf32(-1, 1), -1),
				.velocity = GetRandf32(50, 300),
				.pos = V2Add(GetMousePosition(), Vec2(GetRandf32(-1, 1), GetRandf32(-1, 1))),
				.size = Vec2v(2),
				.duration = GetRandf32(0.3f, 1),
				.color_initial = ColA(255, 0, 0, 255),
				.color_end = ColA(255, 0, 0, 0),
			);
		}
	}
	if (IsKeyPressed(KEY_U)) {
		exit(0);
	}
	#endif

	return (false);
}

internal b32 update_game(void) 
{
	//apply_func_entitys(Level, update_entity_veffects);

	//pply_func_entitys(Level, update_entity_animations);

	return (false);
}

hot void draw(void)
{
	render_entity((Entity *) &Level->player);

	DrawTextEx(Data->font, TextFormat("%d", Level->player.state), Vec2(10, 10), Data->font.baseSize, 2, BLACK);
	DrawTextEx(Data->font, TextFormat("player->velocity: %.3f, %.3f\n", Level->player.velocity.x, Level->player.velocity.y), Vec2(10, 15), Data->font.baseSize, 2, BLACK);
	if (Level->player.state == PlayerStateCharging) {
		V2 start = V2Add(Level->player.pos, V2Scale(Level->player.render.size, 0.5f));
		V2 end = V2Add(start, V2Scale(Level->player.dir, 10));
		DrawLineV(start, end, BLUE);
	}

	for (i32 i = 0; i < length_of(Level->obstacles); i++) {
		if (Level->obstacles[i].type == EntityEmpty) continue;
		render_entity((Entity *) &Level->obstacles[i]);
	}
	collide_player(&Level->player);

	// ---- Ui -----
	MUiRender(Data->ui.mu);

	if (Data->menu_screen) {
		UiContainer *c = &Data->menu;
		XUiBegin(c);

		XUiText(c, "Tower Defense da silva", true);
		
		if (XUiTextButton(c, "Play")) {
			Data->menu_screen = false;
		}

		if (XUiTextButton(c, "Options")) {
			printf("Optiooooons\n");
		}

		XUiEnd(c);
		return ;
	}

	draw_editor();
}

GameFunctions game_init_functions()
{
	return (GameFunctions) {
		.init_pre_raylib = &init_pre_raylib,
		.init_pos_raylib = &init_pos_raylib,
		.update = &update,
		.draw = &draw,
		.pre_reload = &pre_reload,
		.pos_reload = &pos_reload,
	};
}

void collide_player(Player *player)
{
	Obstacles against[10] = {0};
	i32 count = 0;
	for (i32 i = 0; i < length_of(Level->obstacles); i++) {
		Obstacles *ob = &Level->obstacles[i];
		if (ob->type == EntityEmpty) continue;
		if (V2Distance(player->pos, ob->pos) < TILE_SIZE * 2) {
			against[count] = *ob;
			count++;
			if (count >= length_of(against)) {
				TraceLog(LOG_INFO, "collide_player breaking early");
			}
		}
	}
	// Rect player_rec = player->collision;
	// player_rec.x += player->pos.x;
	// player_rec.y += player->pos.y;
	// DrawRectangleRec(player_rec, PURPLE);
	for (i32 i = 0; i < count; i++) {
		V2 contact_p = {0};
		V2 contact_n = {0};
		f32 contact_t = 0;
		Rect target_rec = against[i].collision;
		target_rec.x += against[i].pos.x;
		target_rec.y += against[i].pos.y;
		DrawRectangleRec(target_rec, BLUE);
		if (CheckCollisionDynamicRectRect(player->pos, player->collision, player->velocity, target_rec, &contact_p, &contact_n, &contact_t, GetFrameTime()) && contact_t < 1) {
			V2 vel = {fabsf(player->velocity.x), fabsf(player->velocity.y)};
			player->velocity = V2Add(player->velocity, V2Multiply(contact_n, V2Scale(vel, 1 - contact_t)));
			DrawCircleV(contact_p, 4, RED);
		}
	}
}

void create_obstacle(GameLevel *level, Obstacles ob) 
{
	for (i32 i = 0; i < length_of(level->obstacles); i++) {
		if (level->obstacles[i].type == EntityEmpty) {
			level->obstacles[i] = ob;
			return ;
		}
	}
	TraceLog(LOG_WARNING, "create_obstacle: level obstacles array is full.");
}

void init_level(GameLevel *level)
{
	level->name = "level 1";

	level->player = (Player) {
		.type = EntityPlayer,
		.pos = Vec2(Data->canvas_size.x * 0.5f, Data->canvas_size.y * 0.5f),
		.collision = Rec(0, 0, 8, 15),
		.render = CreateSprite(Data->sheet, .size = Vec2v(16), .tint = GREEN),
	};

	V2 tiles_amount = Vec2(Data->canvas_size.x / TILE_SIZE, Data->canvas_size.y / TILE_SIZE);
	for (i32 i = 0; i < tiles_amount.x; i ++) {
		create_obstacle(level, (Obstacles) {
			.type = EntityStatic,
			.pos = Vec2(i * TILE_SIZE, TILE_SIZE),
			.render = CreateSprite(Data->sheet, .size = Vec2v(16), .tint = GRAY),
			.collision = Rec(0, 0, TILE_SIZE, TILE_SIZE),
		});
	};
	for (i32 i = 0; i < tiles_amount.x; i ++) {
		create_obstacle(level, (Obstacles) {
			.type = EntityStatic,
			.pos = Vec2(i * TILE_SIZE, Data->canvas_size.y - TILE_SIZE),
			.render = CreateSprite(Data->sheet, .size = Vec2v(16), .tint = GRAY),
			.collision = Rec(0, 0, TILE_SIZE, TILE_SIZE),
		});
	};
	
	// V2 grid = Vec2(Data->canvas_size.x / TILE_SIZE, Data->canvas_size.y / TILE_SIZE);
}

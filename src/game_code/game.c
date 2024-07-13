#include "game.h"
#include "../modules/input.h"

static GameData	*Data = NULL;
static V2	pos = {0};

static void	init(GameData *data)
{
	Data = data;
}

static void	update()
{

	if (IsActionDown(RIGHT)) {
		pos.x += 5;
	}
	if (IsActionDown(LEFT)) {
		pos.x -= 5;
	}
	if (IsActionDown(DOWN)) {
		pos.y += 5;
	}
	if (IsActionDown(UP)) {
		pos.y -= 5;
	}
}

static void	draw()
{
	DrawRectangleV(pos, (V2) {100, 100}, RED);
}

static void	pre_reload()
{
}

static void	pos_reload()
{
}

GameFunctions	game_init_functions()
{
	return (GameFunctions) {
		.init = &init,
		.update = &update,
		.draw = &draw,
		.pre_reload = &pre_reload,
		.pos_reload = &pos_reload,
	};
}

#include "game.h"
#include "input.h"
 
#ifdef HOT_RELOAD
# define private
#else
# define private static
#endif

static GameData	*Data = NULL;

private void	init(GameData *data)
{
	Data = data;
}

private void	update()
{
	PoolActions();

	if (IsActionDown(RIGHT)) {
		Data->pos.x += 5;
	}
	if (IsActionDown(LEFT)) {
		Data->pos.x -= 5;
	}
	if (IsActionDown(DOWN)) {
		Data->pos.y += 5;
	}
	if (IsActionDown(UP)) {
		Data->pos.y -= 5;
	}
}

private void	draw()
{
	DrawRectangleV(Data->pos, (V2) {200, 200}, PURPLE);
	DrawRectangleV(Data->pos, (V2) {100, 100}, YELLOW);
}

private void	pre_reload()
{
}

private void	pos_reload(GameData *data)
{
	Data = data;
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


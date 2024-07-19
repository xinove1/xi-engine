#include "game.h"
#include "input.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
 
#ifdef HOT_RELOAD
# define private
#else
# define private static
#endif

static GameData  *Data = NULL;
static GameLevel *Level= NULL;
static PlayerEntity *Player= NULL;

private void	init(GameData *data)
{
	Data = data;

	{
		V2 map_size = {15, 10};
		int map_size_int = map_size.x * map_size.y;
		GameLevel *level = malloc(sizeof(GameLevel));
		*level = (GameLevel) {
			.name = "Test Level",
			.map = malloc(sizeof(int) * map_size_int),
			.map_sz = map_size,
			.entitys = malloc(sizeof(Entity*) * map_size_int),
			.max_entitys = map_size_int,
			.player = (PlayerEntity) {
				.type = EntityPlayer,
				.pos = (V2) { map_size.x / 2, map_size.y / 2},
				.color = BLUE,
			},
		};
		memset(level->map, -1, map_size_int);
		memset(level->entitys, 0, map_size_int);
		Player = &level->player;
		Level = level;
		data->current_level = level;
	}

}

private void	update()
{
	assert(Data && Level && Player);
	PoolActions();

	V2	dir = {0, 0};
	if (IsActionPressed(RIGHT)) {
		dir.x += 1;
	}
	if (IsActionPressed(LEFT)) {
		dir.x -= 1;
	}
	if (IsActionPressed(DOWN)) {
		dir.y += 1;
	}
	if (IsActionPressed(UP)) {
		dir.y -= 1;
	}
	Player->look_dir = dir;

	// Resolve Collisions
	
	Player->pos = V2Add(Player->pos, dir);
}

private void	draw()
{
	DrawRectangleV(V2Scale(Player->pos, TILE), (V2) {TILE, TILE}, Player->color);
}

private void	pre_reload()
{
}

private void	pos_reload(GameData *data)
{
	Data = data;
	Level = data->current_level;
	Player = &data->current_level->player;
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

static int get_map_pos(GameLevel *level, V2 pos);
static void set_map_pos(GameLevel *level, V2 pos, int value);

static bool move_entity(GameLevel *level, Entity *e, V2 where)
{
	bool moved = false;
	V2	size = level->map_sz;
	if (where.x < 0 || where.x >= size.x ||
	    where.y < 0 || where.y >= size.y) {
		TraceLog(LOG_WARNING, "Trying to Move Entity to outside of map bounds.");
		return (false);
	}
	if (get_map_pos(level, where) != -1) {
		TraceLog(LOG_WARNING, "Trying to Move Entity to an occupied place.");
		return (false);
	}
	set_map_pos(level, e->pos, -1);
	set_map_pos(level, where, -1);

	return (moved);
}

static int get_map_pos(GameLevel *level, V2 pos)
{

}

static void set_map_pos(GameLevel *level, V2 pos, int value)
{

}

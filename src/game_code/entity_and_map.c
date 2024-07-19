#include "game.h"

bool move_entity(GameLevel *level, Entity *e, V2 where)
{
	assert(level && e);
	bool moved = true;
	V2	size = level->map_sz;
	if (where.x < 0 || where.x >= size.x ||
	    where.y < 0 || where.y >= size.y) {
		TraceLog(LOG_WARNING, "move_entity: Trying to Move Entity to outside of map bounds.");
		return (false);
	}
	if (get_map_pos(level, where) != -1) {
		TraceLog(LOG_WARNING, "move_entity: Trying to Move Entity to an occupied place.");
		return (false);
	}
	if (e->type == EntityActuator) {
		TraceLog(LOG_WARNING, "move_entity: Moving Actuators via this function is not allowed.");
		return (false);
	}
	set_map_pos(level, e->pos, -1);
	set_map_pos(level, where, e->id);
	e->pos = where;
	return (moved);
}

bool move_entity_swap(GameLevel *level, Entity *e, V2 where) 
{
	assert(level && e);
	bool moved = true;
	V2	size = level->map_sz;
	if (where.x < 0 || where.x >= size.x ||
	    where.y < 0 || where.y >= size.y) {
		TraceLog(LOG_WARNING, "move_entity: Trying to Move Entity to outside of map bounds.");
		return (false);
	}
	if (get_map_pos(level, where) != -1) {
		Entity *o = get_map_entity(level, where);
		o->pos = e->pos;
		e->pos = where;
		set_map_pos(level, where, e->id);
		set_map_pos(level, o->pos, o->id);
	} else {
		set_map_pos(level, e->pos, -1);
		set_map_pos(level, where, e->id);
		e->pos = where;
	}
	return (moved);
}

int get_map_pos(GameLevel *level, V2 pos)
{
	assert(level);
	if (pos.x < 0 || pos.x >= level->map_sz.x || pos.y < 0 || pos.y >= level->map_sz.y) {
		TraceLog(LOG_FATAL, "get_map_pos: Pos outside of map bounds %f, %f", pos.x, pos.y);
	}
	return (level->map[(int) ((pos.y * level->map_sz.x) + pos.x)] );
}

void set_map_pos(GameLevel *level, V2 pos, int value)
{
	assert(level);
	if (pos.x < 0 || pos.x >= level->map_sz.x || pos.y < 0 || pos.y >= level->map_sz.y) {
		TraceLog(LOG_FATAL, "set_map_pos: Pos outside of map bounds %f, %f", pos.x, pos.y);
	}
	level->map[(int) ((pos.y * level->map_sz.x) + pos.x)] = value;
}

Entity *get_map_entity(GameLevel *level, V2 pos)
{
	assert(level);
	int id = get_map_pos(level, pos);
	if (id == -1) return NULL;
	return (get_entity(level, id));
}

Entity *get_entity(GameLevel *level, int entity_id)
{
	assert(level);
	assert(entity_id >= 0 && entity_id < level->max_entitys);
	return (&level->entitys[entity_id]);
}

// TODO  Actually deal with a EntityEmpty in other places, such as create entity
void	delete_entity(GameLevel *level, Entity *e)
{
	assert(level && e);
	if (e->type == EntityActuator) {
		return ;
	}
	e->type = EntityEmpty;
	set_map_pos(level, e->pos, -1);
}

Entity *get_actuator(GameLevel *level, V2 pos) 
{
	assert(level);
	for (int i = 0; i < level->actuators_count; i++) {
		Entity *act = &level->actuators[i];
		if (V2Compare(act->pos, pos)) {
			return (act);
		}
	}

	return (NULL);
}

Entity *create_entity_empty(GameLevel *level) 
{
	assert(level);
	TraceLog(LOG_INFO, "Creating entity, entity_count: %d", level->entity_count);
	Entity *e = NULL;
	e = get_entity(level, level->entity_count);
	int *p = (int *) &e->id; 
	*p = level->entity_count;
	level->entity_count++;
	return (e);
}

Entity *create_entity(GameLevel *level, Entity d) 
{
	assert(level);
	Entity *e = create_entity_empty(level);
	e->type = d.type;
	e->pos = d.pos;
	e->color = d.color;
	e->look_dir = d.look_dir;
	set_map_pos(level, d.pos, e->id);
	return (e);
}

Entity *create_actuator(GameLevel *level, Entity d)
{
	assert(level);
	Entity *e = &level->actuators[level->actuators_count];
	level->actuators_count++;
	if (d.type != EntityActuator) {
		TraceLog(LOG_WARNING, "create_actuator: passed an type that's not EntityActuator, will create using EntityActuator as it's type anyway.");
	}

	e->type = EntityActuator;
	e->id = d.id;
	e->pos = d.pos;
	e->color = d.color;
	e->look_dir = d.look_dir;

	return (e);
}

GameLevel *create_level(char *name, V2 map_size, V2 canvas_size)
{
	V2 map_size_screen = V2Scale(map_size, TILE);
	int map_size_int = map_size.x * map_size.y;
	V2 canvas_middle = V2Scale(canvas_size, 0.5f);
	int max_entitys = map_size_int;

	GameLevel *level = malloc(sizeof(GameLevel));
	*level = (GameLevel) {
		.name = name,
		.map = malloc(sizeof(int) * map_size_int),
		.map_sz = map_size,
		.map_offset = (V2) {canvas_middle.x - (map_size_screen.x * 0.5f), canvas_middle.y - (map_size_screen.y * 0.5f)},
		.entitys = malloc(sizeof(Entity) * max_entitys),
		.max_entitys = max_entitys,
		.entity_count = 0,
		//.actuators = {0},
		.actuators_count = 0,
	};
	memset(level->map, -1, sizeof(int) * map_size_int);
	memset(level->entitys, 0, map_size_int);
	return (level);
}

void print_entity(Entity e)
{
	char *types[EntitysCount + 1] = {"EntityEmpty", "EntityPlayer", "EntityActuator", "EntityMovable", "EntityStatic", "EntitysCount"};
	printf("Entity: \n");
	printf("\t type: %s \n", types[e.type]);
	printf("\t id: %d \n", e.id);
	printf("\t pos: %f, %f \n", e.pos.x, e.pos.y);
	printf("\t look_dir: %f, %f \n", e.pos.x, e.pos.y);
	printf("\t color: %d, %d, %d, %d\n", e.color.r, e.color.g, e.color.b, e.color.a);
}

void print_map(GameLevel *level) 
{
	assert(level);
	printf("Map %s size %f, %f\n", level->name, level->map_sz.x, level->map_sz.y);
	for (int y = 0; y < level->map_sz.y; y++) {
		for (int x = 0; x < level->map_sz.x; x++) {
			printf("%d, ", get_map_pos(level, (V2) {x, y}));
		}
		printf("\n");
	}
}

void print_level(GameLevel *level)
{
	assert(level);
	printf("Level %s \n", level->name);
	printf("Map size: %f, %f \n", level->map_sz.x, level->map_sz.y);
	printf("Map offset: %f, %f \n", level->map_offset.x, level->map_offset.y);
	printf("Max Entitys: %d \n", level->max_entitys);
	printf("Entitys count: %d \n", level->entity_count);
	printf("Actuators count: %d \n", level->actuators_count);
}

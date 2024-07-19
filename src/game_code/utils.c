#include "game.h"
#include <math.h>

V2	ExpDecayV2(V2 a, V2 b, float decay) 
{
	V2	d = Vector2Subtract(a, b);
	d = Vector2Scale(d, expf(-decay * GetFrameTime()));
	return Vector2Add(b, d);
}

inline Rect	RectV2(V2 pos, V2 size) {
	return ((Rect) {.x = pos.x, .y = pos.y, .width = size.x, .height = size.y});
}

inline V2	V2RectPos(Rect rect) {
	return ((V2) {rect.x, rect.y});
}

inline V2	V2RectSize(Rect rect) {
	return ((V2) {rect.width, rect.height});
}

inline bool	V2Compare(V2 a, V2 b) {
	return (a.x == b.x && a.y == b.y);
}

void	draw_grid_ex(V2 position, V2 grid_size, int tile_size, float line_thickness, Color color) 
{
	for (int y = 0; y < grid_size.y + 1; y++) {
		V2	pos = {position.x, position.y + y * (tile_size )};
		V2	end_pos = {position.x + (grid_size.x * tile_size), position.y + y * (tile_size )};
		DrawLineEx(pos, end_pos, line_thickness, color);
	}
	for (int x = 0; x < grid_size.x + 1; x++) {
		V2	pos = {position.x + x * tile_size, position.y};
		V2	end_pos = {position.x + x * tile_size, position.y + grid_size.y * tile_size};
		if (x == 0) {
			// printf("position: %f, %f \n", position.x, position.y);
			// printf("pos: %f, %f \n", pos.x, pos.y);
		}
		DrawLineEx(pos, end_pos, line_thickness, color);
	}
}

void	draw_grid(V2 position, V2 grid_size, int tile_size)
{
	draw_grid_ex(position, grid_size, tile_size, 1, ColorAlpha(BLACK, 0.1));
}


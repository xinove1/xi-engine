#include "game.h"
#include <math.h>

V2 ExpDecayV2(V2 a, V2 b, f32 decay) 
{
	V2 c = V2Subtract(a, b);
	c = V2Scale(c, expf(-decay * GetFrameTime()));
	return V2Add(b, c);
}

f32 ExpDecayf32(f32 a, f32 b, f32 decay) 
{
	f32 c = a - b;
	c = c * expf(-decay * GetFrameTime());
	return (b + c);
}

Color ExpDecayColor(Color a, Color b, f32 decay) 
{
	Color c = ColorSubtract(a, b);
	c = ColorScale(c, expf(-decay * GetFrameTime()));
	return ColorAdd(b, c);
}

Color lerp_color(Color a, Color b, f32 time) 
{
	return ((Color) {
		a.r + (b.r - a.r) * time,
		a.g + (b.g - a.g) * time,
		a.b + (b.b - a.b) * time,
		a.a + (b.a - a.a) * time
	});
}

Rect GetWindowRect()
{
	return (Rec(0, 0, Data->canvas_size.x, Data->canvas_size.y));
}

f32 GetRandf32Ex(f32 min, f32 max, f32 acc) 
{
	f32 r = GetRandomValue(min * acc, max * acc);
	return (r / acc);
}

void draw_grid_ex(V2 position, V2 grid_size, i32 tile_size, f32 line_thickness, Color color) 
{
	for (i32 y = 0; y < grid_size.y + 1; y++) {
		V2 pos = {position.x, position.y + y * (tile_size )};
		V2 end_pos = {position.x + (grid_size.x * tile_size), position.y + y * (tile_size )};
		DrawLineEx(pos, end_pos, line_thickness, color);
	}
	for (i32 x = 0; x < grid_size.x + 1; x++) {
		V2 pos = {position.x + x * tile_size, position.y};
		V2 end_pos = {position.x + x * tile_size, position.y + grid_size.y * tile_size};
		if (x == 0) {
			// printf("position: %f, %f \n", position.x, position.y);
			// printf("pos: %f, %f \n", pos.x, pos.y);
		}
		DrawLineEx(pos, end_pos, line_thickness, color);
	}
}

void draw_grid(V2 position, V2 grid_size, int tile_size)
{
	draw_grid_ex(position, grid_size, tile_size, 1, ColorAlpha(BLACK, 0.1));
}

Font LoadFontFromImageSheet(Image image, Vector2 grid_size, int first_char)
{
    Font font = GetFontDefault();
	
	if (image.width % (int) grid_size.x != 0 || image.height % (int) grid_size.y != 0) {
		TraceLog(LOG_ERROR, "LoadFontFromImageSheet: image can't be perfectly divided by provided grid_size. Note that grid_size floating value is ignored.");
		return (font);
	}

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
	V2 image_grided = {image.width / (int) grid_size.x, image.height / (int) grid_size.y}; // TODO Better name I don't remember the term for this :(
	int glyph_count = image_grided.x * image_grided.y;

	// TODO  Transform all color in image to white so tint can be applied
    Color *pixels = LoadImageColors(image);
    for (int i = 0; i < image.height*image.width; i++) if (!ColorCompare(pixels[i], BLANK)) pixels[i] = WHITE;

    Image image_white = {
        .data = pixels,
        .width = image.width,
        .height = image.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Set font with all data parsed from image
    font.texture = LoadTextureFromImage(image_white); // Convert processed image to OpenGL texture
    font.glyphCount = glyph_count;
    font.glyphPadding = 0;
    font.glyphs = (GlyphInfo *)RL_MALLOC(font.glyphCount*sizeof(GlyphInfo));
    font.recs = (Rectangle *)RL_MALLOC(font.glyphCount*sizeof(Rectangle));

	for (int y = 0; y < image_grided.y; y++) {
		for (int x = 0; x < image_grided.x; x++) {
			int i = x + (y * image_grided.x);
			font.glyphs[i].value = first_char + i;
			font.glyphs[i].offsetX = 0;
			font.glyphs[i].offsetY = 0;
			font.glyphs[i].advanceX = 0;
			font.recs[i] = (Rectangle) {x * (int)grid_size.x, y * (int)grid_size.y, (int) grid_size.x, (int) grid_size.y};
			font.glyphs[i].image = ImageFromImage(image_white, font.recs[i]);
		}
	}
	UnloadImage(image_white);
    font.baseSize = (int)font.recs[0].height;
    return font;
}

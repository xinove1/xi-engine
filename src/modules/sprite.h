#ifndef SPRITE_H_
#define SPRITE_H_
# include "core.h"
# include "raylib.h"
# include "raymath_short.h"

typedef enum {
	SpriteEmpty,
	SpriteWhole,
	SpriteSheeted,
	SpriteAnimation,
} SpriteTypes;

typedef struct {
	SpriteTypes type;
	V2 pos;
	V2 size;
	f32 rotation;
	b32 flipped;
	Color tint;
	Texture2D texture;
	Rect src_rec;
	i32 frame;
	i32 frame_start;
	i32 frame_end;
	f32 frame_duration;
	f32 frame_duration_count;
} Sprite;

typedef struct {
	V2 pos;
	V2 size;
	f32 rotation;
	Color tint;
	b32 flipped;
} CreateSpriteParams;

typedef struct {
	V2 pos;
	V2 size;
	f32 rotation;
	Color tint;
	b32 flipped;
	i32 frame_start;
	i32 frame_end;
	f32 frame_duration;
} CreateSpriteAnimationParams;

void DrawSprite(Sprite sprite);
void UpdateSprite(Sprite *sprite, f32 time_passed);
Sprite CreateSpriteEx(Texture2D texture, CreateSpriteParams params) ;
Sprite CreateSpriteSheetedEx(Texture2D texture, V2 grid_size, i32 frame, CreateSpriteParams params) ;
Sprite CreateSpriteAnimationEx(Texture2D texture, V2 grid_size, CreateSpriteAnimationParams params);
#define CreateSprite(texture, ...) CreateSpriteEx(texture, (CreateSpriteParams)  {.pos = {0}, .size = {texture.width, texture.height}, .rotation = 0, .tint = WHITE, .flipped = 0, __VA_ARGS__})
#define CreateSpriteSheeted(texture, grid_size, frame, ...) CreateSpriteSheetedEx(texture, grid_size, frame, (CreateSpriteParams) {.pos = {0}, .size = {0}, .rotation = 0, .tint = WHITE, .flipped = 0, __VA_ARGS__})
#define CreateSpriteAnimation(texture, grid_size, ...)  CreateSpriteAnimationEx(texture, grid_size, (CreateSpriteAnimationParams) {.pos = {0}, .size = {0}, .rotation = 0, .tint = WHITE, .flipped = 0, .frame_start = 0, .frame_end = 0, .frame_duration = 0.1f, __VA_ARGS__})

#endif

#ifdef SPRITE_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

void UpdateSprite(Sprite *sprite, f32 time_passed) 
{
	if (sprite->type == SpriteAnimation) {
		sprite->frame_duration_count += time_passed;
		if (sprite->frame_duration_count >= sprite->frame_duration) {
			sprite->frame++;
			if (sprite->frame > sprite->frame_end) {
				sprite->frame = sprite->frame_start;
			}
			sprite->frame_duration_count = 0;
		}
	}
}

void DrawSprite(Sprite sprite) 
{

	Texture2D t = sprite.texture;
	if (t.id == 0) {
		DrawRectangleV(sprite.pos, sprite.size, sprite.tint);
		return ;
	}
	if (sprite.type == SpriteEmpty) { return ; }
	Rect rec_src = (sprite.type == SpriteWhole) ? Rec(0, 0, t.width, t.height) : sprite.src_rec;
	Rect rec_dst = RecV2(sprite.pos, sprite.size);


	if ((sprite.type == SpriteSheeted || sprite.type == SpriteAnimation) && sprite.frame > 0) {
		V2 size = {t.width / rec_src.width, t.height / rec_src.height};
		if (size.x <= 0 || size.y <= 0) {
			TraceLog(LOG_ERROR, "DrawSprite: error calculating texture sheet size for SpriteSheeted/SpriteAnimation.");
			return ;
		}
		V2 frame = {sprite.frame - (sprite.frame - (sprite.frame % (i32)size.x)), (i32) (sprite.frame / size.y)}; 
		if (size.x > 1) {
			rec_src.x = frame.x * rec_src.width;
		}
		if (size.y > 1) {
			rec_src.y = frame.y * rec_src.height;
		}
	}

	if (sprite.flipped) {
		rec_src.width = -rec_src.width;
	}
	DrawTexturePro(t, rec_src, rec_dst, Vec2v(0), sprite.rotation, sprite.tint);
}

Sprite CreateSpriteEx(Texture2D texture, CreateSpriteParams p) 
{
	Sprite r = (Sprite) {
		.type = SpriteWhole,
		.pos = p.pos,
		.size = p.size,
		.tint = p.tint,
		.rotation = p.rotation,
		.flipped = p.flipped,
		.texture = texture,
	};
	return (r);
}

Sprite CreateSpriteSheetedEx(Texture2D texture, V2 grid_size, i32 frame, CreateSpriteParams p)
{
	if (p.size.x == 0 && p.size.y == 0) {p.size = grid_size;};
	Sprite r = (Sprite) {
		.type = SpriteSheeted,
		.pos = p.pos,
		.size = p.size,
		.tint = p.tint,
		.rotation = p.rotation,
		.flipped = p.flipped,
		.texture = texture,
		.frame = frame,
		.src_rec = Rec(0, 0, grid_size.x, grid_size.y),
	};
	return (r);
}

Sprite CreateSpriteAnimationEx(Texture2D texture, V2 grid_size, CreateSpriteAnimationParams p)
{
	if (p.size.x == 0 && p.size.y == 0) {p.size = grid_size;};
	i32 frame_end = p.frame_end;
	if (frame_end == 0) {
		frame_end = (texture.width / grid_size.x) * (texture.height / grid_size.y) - 1;
	}
	Sprite r = (Sprite) {
		.type = SpriteAnimation,
		.pos = p.pos,
		.size = p.size,
		.tint = p.tint,
		.rotation = p.rotation,
		.flipped = p.flipped,
		.frame = p.frame_start,
		.frame_start = p.frame_start,
		.frame_duration = p.frame_duration,
		.frame_end = frame_end,
		.frame_duration_count = 0,
		.texture = texture,
		.src_rec = Rec(0, 0, grid_size.x, grid_size.y),
	};
	return (r);
}

// NOLINTEND(misc-definitions-in-headers)
#endif

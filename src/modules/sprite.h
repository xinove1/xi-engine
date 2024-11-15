#ifndef SPRITE_H_
#define SPRITE_H_
# include "core.h"
# include "raylib.h"
# include "raymath_short.h"

typedef struct {
	Texture2D texture;
	Rect rect;
} SpriteTexture;

typedef struct {
	V2 pos;
	V2 size;
	f32 rotation;
	Color tint;
	i32 frame;
	b32 flipped;
	SpriteTexture texture;
} Sprite;

typedef struct {
	V2 pos;
	V2 size;
	f32 rotation;
	Color tint;
	i32 frame;
	b32 flipped;
} CreateSpriteParams;

inline void DrawSprite(Sprite sprite);
Sprite CreateSpriteEx(SpriteTexture texture, CreateSpriteParams params) ;
#define CreateSprite(texture, ...) CreateSpriteEx(texture, (CreateSpriteParams) {.pos = Vec2v(0), .size = Vec2(texture.rect.width, texture.rect.height), .rotation = 0, .tint = WHITE, .frame = 0, .flipped = 0, __VA_ARGS__})

#endif

#ifdef SPRITE_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

void DrawSprite(Sprite sprite) 
{
	Texture2D texture = sprite.texture.texture;
	if (texture.id > 0) {
		Rect src_rec = sprite.texture.rect;
		if (sprite.frame > 0) {
			V2 size = {texture.width / src_rec.width, texture.height / src_rec.height};
			V2 frame = {sprite.frame - (sprite.frame - (sprite.frame % (i32)size.x)), (u32) (sprite.frame / size.y)}; 
			//TraceLog(LOG_INFO, "frame: %f, %f \n", frame.x, frame.y);
			src_rec.x = frame.x * src_rec.width;
			src_rec.y = frame.y * src_rec.height;
		}
		if (sprite.flipped) {
			src_rec.x = src_rec.x + src_rec.width;
			src_rec.y = src_rec.y + src_rec.height;
			src_rec.width = -src_rec.width;
		}
		DrawTexturePro(texture, src_rec, RecV2(sprite.pos, sprite.size), Vec2v(0), sprite.rotation, sprite.tint);
	} else { 
		DrawRectangleV(sprite.pos, sprite.size, sprite.tint);
	}
}

Sprite CreateSpriteEx(SpriteTexture texture, CreateSpriteParams p) 
{
	Sprite r = (Sprite) {
		.pos = p.pos,
		.size = p.size,
		.tint = p.tint,
		.frame = p.frame,
		.rotation = p.rotation,
		.flipped = p.flipped,
		.texture = texture,
	};
	return (r);
}

// NOLINTEND(misc-definitions-in-headers)
#endif

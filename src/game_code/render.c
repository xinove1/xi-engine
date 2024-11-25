#include "game.h"

void render_particle(Particle p)
{
	if (p.type == ParticleEmpty) return ;
	Rect rec = RecV2(p.pos, p.size);
	Color color = lerp_color(p.color_initial, p.color_end, p.duration_count / p.duration);
	DrawRectangleRec(rec, color);
}

void render_entity(Entity *entity)
{
	if (entity->type == EntityEmpty) return ;

	Sprite sprite = entity->render;
	sprite.pos = V2Add(entity->pos, entity->render.pos);
	DrawSprite(sprite);

	#ifdef BUILD_DEBUG
	Color collision_debug = ColorAlpha(RED, 0.2f);
	V2 pos = V2Add(entity->pos, Vec2(entity->collision.x, entity->collision.y));
	// DrawRectangleV(pos, Vec2(entity->collision.width, entity->collision.height), collision_debug);
	#endif

	return ;
}

void render_env_sprites(EnvSprite *arr, i32 arr_size)
{
	for (i32 i = 0; i < arr_size; i++) {
		EnvSprite *s = &arr[i];
		if (s->type == EnvSpriteEmpty) break;
		Sprite sprite = s->sprite;
		sprite.pos = V2Add(s->pos, sprite.pos);
		DrawSprite(sprite);
	}
}

void update_entity_animations(Entity *entity)
{
	UpdateSprite(&entity->render, GetFrameTime());
}

void update_entity_veffects(Entity *entity) 
{
	for (i32 i = 0; i < VEffectTypeCount; i++) {
		VEffect *effect = &entity->veffects[i];
		if (effect->finished) continue;
		Sprite *r = &entity->render;
		switch (effect->type) {
			case VEffectEmpty: case VEffectTypeCount: continue;
			case VEffectFlashColor: {
				if (effect->duration_count == 0 ) { // First pass
					//r->color = effect->color_target;
				}
				// TODO  Change so it will not wait for previous effect to finish, and have a more gradual sloope returning to original color
				f32 percent = effect->duration_count / effect->duration;
				f32 percent_cutoff = 0.25f;
				if (percent < percent_cutoff) {
					percent = f32Remap(percent, 0, percent_cutoff, 0, 1.f);
					r->tint = lerp_color(r->tint, effect->color_target, percent);
				} else {
					percent = f32Remap(percent, percent_cutoff, 1.f, 0, 1.f);
					 r->tint = lerp_color(r->tint, effect->color_original, percent);
				}
				effect->duration_count += GetFrameTime();
				if (effect->duration_count >= effect->duration ) { // Last pass
					r->tint = effect->color_original;
					effect->finished = true;
				}
			} break ;
			case VEffectShake: {
				if (effect->duration_count == 0 ) { // First pass
				}
				# define rng(min, max) GetRandomValue(min, max)
				
				r->pos = V2Scale(Vec2(rng(-1, 1), rng(-1, 1)), GetRandomValue(0, 1));

				effect->duration_count += GetFrameTime();
				if (effect->duration_count >= effect->duration ) { // Last pass
					r->pos = V2Zero();
					effect->finished = true;
				}
			} break ;

			default: {
				TraceLog(LOG_WARNING, "update_entity_veffects: effect %s not implemented yet.", VEffectTypeNames[effect->type]);
			} break ;
		}
	}
}

void apply_flash_effect(Entity *entity, Color color, f32 duration) 
{
	VEffect *effect = &entity->veffects[VEffectFlashColor];
	if (effect->finished || effect->type == VEffectEmpty) {
		*effect = (VEffect) {
			.type = VEffectFlashColor,
			.duration = duration,
			.color_target = color,
			.color_original = entity->render.tint,
		};
	}
}

void apply_shake_effect(Entity *entity, f32 duration) 
{
	VEffect *effect = &entity->veffects[VEffectShake];
	if (effect->finished || effect->type == VEffectEmpty) {
		*effect = (VEffect) {
			.type = VEffectShake,
			.duration = duration,
		};
	}
}

void create_particle_ex(CreateParticleParams param)
{
	Particle p = (Particle) {
		.type = ParticleAlive,
		.dir = param.dir,
		.velocity = param.velocity,
		.pos = param.pos,
		.size = param.size,
		.duration = param.duration,
		.color_initial = param.color_initial,
		.color_end = param.color_end,
	};
	for (i32 i = 0; i < count_of(Data->particles); i++) {
		if (Data->particles[i].type == ParticleEmpty) {
			Data->particles[i] = p;
			break ;
		}
	}
}

void create_env_sprite(EnvSprite *arr, i32 arr_size, EnvSprite env_sprite)
{
	for (i32 i = 0; i < arr_size; i++) {
		EnvSprite *s = &arr[i];
		if (s->type == EnvSpriteEmpty) {
			*s = env_sprite;
			return ;
		}
	}
	TraceLog(LOG_WARNING, "create_env_sprite: can't add env_sprite to array, it's full\n");
}


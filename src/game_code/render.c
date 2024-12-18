#include "game.h"

void render_particle(Particle p)
{
	if (p.type == ParticleEmpty) return ;
	Rect rec = RecV2(p.pos, p.size);
	Color color = lerp_color(p.color_initial, p.color_end, p.duration_count / p.duration);
	DrawRectangleRec(rec, color);
}

void render_entity(GenericEntity *entity)
{
	if (entity->type == EntityEmpty) return ;

	Sprite sprite = entity->render;
	sprite.pos = V2Add(entity->pos, entity->render.pos);
	DrawSprite(sprite);
	// Rect rec = RecV2(V2Add(entity->pos, entity->render.pos), entity->render.size);
	// DrawRectangleRec(rec, entity->render.tint);

	// cstr *text = TextFormat("%d", entity->floor);
	// i32 text_size = MeasureText(text, 10);
	// DrawText(text, entity->pos.x, entity->pos.y - text_size * 2, 10, RED);

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

void draw_health_bar(GenericEntity *entity) 
{
	if (entity->health_max == 0 || entity->type == EntityProjectile) {
		//printf("%s has health_max as zero. \n", EntityTypeNames[entity->type]);
		return ;
	}
	Rect rec = RecV2(V2Add(entity->pos, entity->render.pos), entity->render.size);
	V2 health_size = Vec2(6, 3);
	V2 health_pos = RecPos(rec);
	health_pos.y -= health_size.y + 3;
	f32 current_health_scalar = entity->health / entity->health_max;
	Rect health_max = RecV2(health_pos, health_size);
	Rect health_current = RecV2(health_pos, Vec2(health_size.x * current_health_scalar, health_size.y * 0.9f));
	DrawRectangleRec(health_max, BLACK);
	DrawRectangleRec(health_current, RED);
}

void update_entity_animations(GenericEntity *entity)
{
	UpdateSprite(&entity->render, GetFrameTime());
}

void update_entity_veffects(GenericEntity *entity) 
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

void apply_flash_effect(GenericEntity *entity, Color color, f32 duration) 
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

void apply_shake_effect(GenericEntity *entity, f32 duration) 
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


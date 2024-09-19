#include "game.h"

void apply_effects(EffectDa da)
{
	{effects_iterate(da) {
		Effect *e = iterate_get();
		if (e->type == EffectEmpty) continue;
		if (e->target->type == EntityEmpty) {
			printf("effect target is empty\n");
			*e = (Effect) {0}; continue;
		}
		//printf("effect_type: %s, target type: %s \n", EffectTypeNames[e->type], EntityTypeNames[e->target->type]);

		e->duration_count += GetFrameTime();
		if (e->duration_count >= e->duration) {
			printf("effect finished. \n");
			printf("size of entity: %ld finished. \n", sizeof(Entity));
			printf("data_offset: %ld, data_size %ld.\n", e->data_offset, e->data_size);
			void *p = e->target + e->data_offset;
			assert(p);
			memcpy(p, e->data, e->data_size);
			*e = (Effect) {0}; continue;
		}

	}}
}

void push_effect(EffectDa *da, Effect effect) 
{
	assert(da);

	EffectDa _da = *da;
	{effects_iterate(_da) {
		Effect *e = iterate_get();
		if (e->type == EffectEmpty) {
			*e = effect;
			return ;
		}
	}}

	if (da->count < da->capacity) {
		da->items[da->count] = effect;
		da->count++;
	} else {
		TraceLog(LOG_WARNING, "push_effect: EffectDa is full.");
	}
}

Effect create_flash_effect(Entity *target, f32 duration, Color color, void *data_offset, void *data, size data_size) 
{
	assert(target);
	if (data_offset && (data_offset < (void*) target ||  data_offset > (void*) target + sizeof(Entity))) {
		TraceLog(LOG_WARNING, "create_flash_effect: data_offset of target is invalid, creating stub effect.");
		return ((Effect) {0});
	}

	size offset = data_offset - (void*) target;
	assert(offset + data_size < sizeof(Entity));

	Effect effect =  {
		.type = EffectFlashColor,
		.duration = duration,
		.target = target,
		.data_offset = offset,
		.data_size = data_size,
		.flash.color_target = target->color,
		.flash.color_flash = color,
	};

	memcpy(effect.data, data, data_size);

	return (effect);
}

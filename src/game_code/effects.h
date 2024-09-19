#ifndef EFFECTS_H_
# define EFFECTS_H_
# include "core.h"
# include "entitys.h"

// TODO  Separate to own header
typedef enum {EffectEmpty, EffectFlashColor, EffectShake, EffectTypeCount} EffectType;
global cstr *EffectTypeNames[] = {"EffectEmpty", "EffectFlashColor", "EffectShake", "EffectTypeCount"};

typedef struct {
	EffectType type;
	f32 duration;
	f32 duration_count;
	Entity *target;
	byte data[sizeof(Entity)];
	size data_offset;
	size data_size;
	// to be applied to target once duration is due
	// if theres an effect that the duration is so big that other data changend during the duration and we are overrind
	// we could try just storing a mem offsets and what to but there with a Da
	union {
		struct {
			Color color_target;
			Color color_flash;
		} flash;
	};
} Effect;

typedef struct {
	Effect *items;
	size count;
	size capacity;
} EffectDa;

#endif

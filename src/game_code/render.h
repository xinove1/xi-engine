#ifndef RENDER_H_
# define  RENDER_H_
# include "core.h"

typedef enum {VEffectEmpty, VEffectFlashColor, VEffectShake, VEffectTypeCount} VEffectType;
global cstr *VEffectTypeNames[] = {"VEffectEmpty", "VEffectFlashColor", "VEffectShake", "VEffectTypeCount"};

typedef struct {
	VEffectType type;
	bool finished;
	f32 duration;
	f32 duration_count;
	union {
		// Flash
		struct {
			Color color_original;
			Color color_target;
		};
	};
} VEffect;

typedef struct {
	V2 pos;
	V2 size;
	Color color;
} RenderData;

#endif 

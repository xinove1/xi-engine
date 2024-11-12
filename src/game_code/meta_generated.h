#ifndef META_GEN_H_
# define META_GEN_H_

typedef enum { 
	 MetaType_EntityType,
	 MetaType_RenderData,
	 MetaType_VEffect,
	 MetaType_TODO,
	 MetaType_V2,
	 MetaType_f32,
	 MetaTypeCount, 
} MetaTypes; 

typedef struct { 
	 MetaTypes type;
	 cstr *name;
	 size offset;
} MetaMember; 

global cstr *EntityTypeNames[] = { 
	 "EntityEmpty",
	 "EntityTurret",
	 "EntityCake",
	 "EntityProjectile",
	 "EntityEnemy",
	 "EntityTurretSpot",
	 "EntityTypeCount",
}; 

global MetaMember MembersOf_GenericEntity[] = { 
	{MetaType_EntityType, "type", offset_of(GenericEntity, type)},
	{MetaType_RenderData, "render", offset_of(GenericEntity, render)},
	{MetaType_VEffect, "veffects", offset_of(GenericEntity, veffects)},
	{MetaType_TODO, "pos", offset_of(GenericEntity, pos)},
	{MetaType_V2, "size", offset_of(GenericEntity, size)},
	{MetaType_f32, "health", offset_of(GenericEntity, health)},
	{MetaType_f32, "health_max", offset_of(GenericEntity, health_max)},
}; 

global cstr *VEffectTypeNames[] = { 
	 "VEffectEmpty",
	 "VEffectFlashColor",
	 "VEffectShake",
	 "VEffectTypeCount",
}; 

global cstr *ParticleTypeNames[] = { 
	 "ParticleEmpty",
	 "ParticleAlive",
	 "ParticleTypeCount",
}; 


#endif 
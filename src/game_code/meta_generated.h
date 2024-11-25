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

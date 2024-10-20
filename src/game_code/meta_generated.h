#ifndef META_GEN_H_
# define META_GEN_H_

typedef enum { 
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
	 "EntityMainTower",
	 "EntityProjectile",
	 "EntityEnemy",
	 "EntityEnemySpawner",
	 "EntityTypeCount",
}; 

global cstr *VEffectTypeNames[] = { 
	 "VEffectEmpty",
	 "VEffectFlashColor",
	 "VEffectShake",
	 "VEffectTypeCount",
}; 


#endif 
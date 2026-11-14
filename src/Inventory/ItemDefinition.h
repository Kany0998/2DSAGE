#ifndef ITEMDEFINITION_H
#define ITEMDEFINITION_H

#include "../Components/AttributesComponent.h"
#include <string>

struct  WeaponStats {
	int minDamage = 1;
	int maxDamage = 5;
	std::string projectileTexture = "bullet-texture";
	float projectileSpeed = 400.0f;
	float shotPerSecond = 2.0f;
	float rangeTiles = 6.0f;
	std::string shotSound;
	int pelletCount = 1;          // 1 = single shot
	float spreadDegrees = 0.0f;   // total width of the cone
};


struct ItemDefinition {
	std::string id;
	std::string name;
	std::string tag;
	std::string icon;
	std::string bag;
	int bonus[Skill_Count] = {};		//zeroed, then filled from Lua
	bool isWeapon = false;
	WeaponStats weapon;			//only meaningful when isWeapon
	std::string ability;		//only for special_ability items
};




#endif // !ITEMDEFINITION_H
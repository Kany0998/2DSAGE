#ifndef  ATTRIBUTESCOMPONENT_H
#define  ATTRIBUTESCOMPONENT_H

#include <iterator>

// Every attribute an entity has, in the order the character sheet lists them. Used as the
// index into AttributesComponent's base/bonus arrays and ProgressionComponent's spentPoints,
// and Skill_Count doubles as their size - so adding a stat here is the only edit needed.
enum SkillStat
{
	Skill_Attack,		//Projectile damage: rolled min..max * (1 + attack / 25.0), rounded, at least 1 (ProjectileEmitSystem)
	Skill_Defense,		//Subtracted from every hit taken; a hit never does less than 1 damage (DamageSystem)
	Skill_Wisdom,		//Mana regeneration per second: 1 + wisdom / 10.0 (ManaRegenerationSystem)
	Skill_Vitality,		//Health regeneration per second: 1 + vitality / 10.0 (HealthRegenerationSystem)
	Skill_Speed,		//Velocity multiplier: 1 + speed / 50.0, floored at 0.1 (MovementSystem)
	Skill_Dexterity,	//Player shots per second: weapon rate * (1 + dexterity / 75.0), floored at 0.1 (ProjectileEmitSystem)
	Skill_Count
};

constexpr const char* SkillLuaNames[] = { "attack", "defense", "wisdom", "vitality", "speed", "dexterity" };
static_assert(std::size(SkillLuaNames) == Skill_Count, "one Lua name per SkillStat");

struct AttributesComponent
{
	//Lua value plus skill points - progression writes here
	int base[Skill_Count];

	//Equipment and later buffs. The rule for whoever writes here: rebuild the whole array
	//from zero whenever one of its sources changes (e.g. sum every equipped item after any
	//equip or unequip). Never adjust it with += on equip and -= on unequip - one missed or
	//doubled path would leave a stat wrong for good, and nothing would ever correct it.
	int bonus[Skill_Count];

	AttributesComponent(int attackPower = 10, int defensePower = 0, int wisdomPower = 10, int vitalityPower = 10, int speedPower = 10, int dexterityPower = 10)
	{
		base[Skill_Attack] = attackPower;
		base[Skill_Defense] = defensePower;
		base[Skill_Wisdom] = wisdomPower;
		base[Skill_Vitality] = vitalityPower;
		base[Skill_Speed] = speedPower;
		base[Skill_Dexterity] = dexterityPower;
		
		//a plain int array in a struct starts with garbage, so zero it
		for (int i = 0; i < Skill_Count; i++) {
			bonus[i] = 0;
		}
	}

	//The only value gameplay reads
	int Effective(SkillStat stat) const {
		return base[stat] + bonus[stat]; 
	}
};

#endif // ! ATTRIBUTESCOMPONENT_H

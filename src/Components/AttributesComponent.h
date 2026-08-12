#ifndef  ATTRIBUTESCOMPONENT_H
#define  ATTRIBUTESCOMPONENT_H

struct AttributesComponent
{
	int attackPower;	// The amount of damage the entity can deal to others
	int defensePower;	// The amount of damage the entity can resist from others
	int wisdomPower;	// The amount of mana regeneration (1 + wisdomPower/10)per second
	int vitalityPower;	// The amount of health regeneration (1 + vitalityPower/10)per second
	int speedPower;		// The speed at which the entity can move
	int dexterityPower;	// The amount of attack speed entity can have (1 + dexterityPower/10) per second

	AttributesComponent(int attackPower = 10, int defensePower = 0, int wisdomPower = 10, int vitalityPower = 10, int speedPower = 10, int dexterityPower = 10)
	{
		this->attackPower = attackPower;
		this->defensePower = defensePower;
		this->wisdomPower = wisdomPower;
		this->vitalityPower = vitalityPower;
		this->speedPower = speedPower;
		this->dexterityPower = dexterityPower;
	}
};

#endif // ! ATTRIBUTESCOMPONENT_H

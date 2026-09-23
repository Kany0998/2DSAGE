#ifndef  MANACOMPONENT_H
#define  MANACOMPONENT_H

struct ManaComponent
{
	int manaPoints = 100;
	int maxManaPoints = 100;
	float manaRegenAccumulator = 0.0f; // Accumulates mana regeneration over time becuse deltaTime is fraction it is needed for mana regen to work in case of ints

	ManaComponent(int manaPoints = 0, int maxManaPoints = 100)
	{
		this->manaPoints = manaPoints;
		this->maxManaPoints = maxManaPoints;
	}
};

#endif // ! MANACOMPONENT_H

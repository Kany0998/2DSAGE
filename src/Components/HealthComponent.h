#ifndef  HEALTHCOMPONENT_H
#define  HEALTHCOMPONENT_H

struct HealthComponent
{
	int healthPoints = 100;
	int maxHealthPoints = 100;
	float healthRegenAccumulator = 0.0f; // Accumulates health regeneration over time becuse deltaTime is fraction it is needed for health regen to work in case of ints

	HealthComponent(int healthPoints = 0, int maxHealthPoints = 100)
	{
		this->healthPoints = healthPoints;
		this->maxHealthPoints = maxHealthPoints;
	}
};

#endif // ! HEALTHCOMPONENT_H

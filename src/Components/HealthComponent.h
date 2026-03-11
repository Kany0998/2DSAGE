#ifndef  HEALTHCOMPONENT_H
#define  HEALTHCOMPONENT_H

struct HealthComponent
{
	int healthPercentage = 100;
	HealthComponent(int healthProcentage = 0)
	{
		this->healthPercentage = healthProcentage;
	}
};

#endif // ! HEALTHCOMPONENT_H

#ifndef  PROJECTILECOMPONENT_H
#define  PROJECTILECOMPONENT_H

#include <SDL.h>

struct ProjectileComponent
{
	bool isFriendly;
	int projectileDamage;
	int duration;
	double startTime;
	bool haveCollided = false;


	ProjectileComponent(bool isFriendly = false, int projectileDamage = 0, int duration = 0, bool haveCollided = false)
	{
		this->isFriendly = isFriendly;
		this->projectileDamage = projectileDamage;
		this->duration = duration;
		this->startTime = SDL_GetTicks();
		this->haveCollided = haveCollided;

	}
};

#endif // ! PROJECTILECOMPONENT_H

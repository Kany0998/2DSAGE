#ifndef  PROJECTILECOMPONENT_H
#define  PROJECTILECOMPONENT_H

#include <SDL.h>

struct ProjectileComponent
{
	bool isFriendly;
	int hitPercentDamage;
	int duration;
	double startTime;

	ProjectileComponent(bool isFriendly = false, int hitPercentDamge = 0, int duration = 0)
	{
		this->isFriendly = isFriendly;
		this->hitPercentDamage = hitPercentDamge;
		this->duration = duration;
		this->startTime = SDL_GetTicks();
	}
};

#endif // ! PROJECTILECOMPONENT_H

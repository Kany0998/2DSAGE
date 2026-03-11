#ifndef PROJECTILEEMITTERCOMPONENT_H
#define PROJECTILEEMITTERCOMPONENT_H

#include <glm/glm.hpp>
#include <SDL.h>

struct ProjectileEmitterComponent
{
	glm::vec2 projectileVelocity;
	int repeatRate; //in milliseconds
	int projectileDuration; //in milliseconds
	int hitPercentDamage;
	int lastEmittedTime; //timestamp in milliseconds
	bool isFriendly; //true if fired by player false if fired by enemy

	ProjectileEmitterComponent(glm::vec2 projectileVelocity = glm::vec2(0), int repeatRate = 0, int projectileDuration = 10000, int hitPercentDamage = 10, bool isFriendly = false)
	{
		this->projectileVelocity = projectileVelocity;
		this->repeatRate = repeatRate;
		this->projectileDuration = projectileDuration;
		this->hitPercentDamage = hitPercentDamage;
		this->isFriendly = isFriendly;
		this->lastEmittedTime = SDL_GetTicks();
	}
};
#endif

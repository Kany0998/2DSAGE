#ifndef  PROJECTILECOMPONENT_H
#define  PROJECTILECOMPONENT_H

#include "../ECS/ECS.h"
#include <SDL.h>

struct ProjectileComponent
{
	bool isFriendly;
	int projectileDamage;
	int duration;
	double startTime;
	bool haveCollided = false;

	// Who fired this shot. Kept so a kill can be attributed back to the shooter
	// (experience rewards, kill feeds, damage credit) instead of being assumed to
	// belong to whoever is tagged "player". Default-constructed for projectiles
	// with no shooter, and NOT safe to dereference blindly - the shooter can die
	// while its bullet is still in flight, so check owner.IsAlive() first.
	Entity owner;

	ProjectileComponent(bool isFriendly = false, int projectileDamage = 0, int duration = 0, Entity owner = Entity(), bool haveCollided = false)
	{
		this->isFriendly = isFriendly;
		this->projectileDamage = projectileDamage;
		this->duration = duration;
		this->startTime = SDL_GetTicks();
		this->owner = owner;
		this->haveCollided = haveCollided;

	}
};

#endif // ! PROJECTILECOMPONENT_H

#ifndef ENTITYKILLEDEVENT_H
#define ENTITYKILLEDEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

// Emitted the moment an entity's health reaches zero, before Entity::Kill() queues
// its destruction. Anything that needs to react to a death subscribes here:
// experience rewards first, loot drops and kill feeds later.
class EntityKilledEvent : public Event
{
	public:
		Entity victim;

		// Whoever fired the killing shot, taken from ProjectileComponent::owner.
		// May be a default-constructed Entity (no known killer), and may refer to an
		// entity that has since died - always check killer.IsAlive() before reading
		// components off it.
		Entity killer;

		// Carried as a plain value rather than read back off the victim later: by the
		// time a subscriber runs, the victim may already have been destroyed by
		// Registry::Update(), and an int has no lifetime problem at all.
		int experienceReward;

		EntityKilledEvent(Entity victim, Entity killer, int experienceReward)
			: victim(victim), killer(killer), experienceReward(experienceReward) {}
};

#endif

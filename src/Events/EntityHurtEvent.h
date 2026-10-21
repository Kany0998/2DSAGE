#ifndef ENTITYHURTEVENT_H
#define ENTITYHURTEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

class EntityHurtEvent : public Event {
	public:
		Entity victim;

		EntityHurtEvent(Entity victim) {
			this->victim = victim;
		}
};


#endif // !ENTITYHURTEVENT_H

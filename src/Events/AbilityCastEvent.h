#ifndef ABILITYCASTEVENT_H
#define ABILITYCASTEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

class AbilityCastEvent : public Event {
	
	public:
		Entity caster;

		
		AbilityCastEvent(Entity caster) {
			this->caster = caster;
		}
};


#endif // !ABILITYCASTEVENT_H

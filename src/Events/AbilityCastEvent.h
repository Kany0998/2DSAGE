#ifndef ABILITYCASTEVENT_H
#define ABILITYCASTEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"
#include <string>

class AbilityCastEvent : public Event {
	
	public:
		Entity caster;
		std::string soundId;
		
		AbilityCastEvent(Entity caster, const std::string& soundId) {
			this->caster = caster;
			this->soundId = soundId;
		}
};


#endif // !ABILITYCASTEVENT_H

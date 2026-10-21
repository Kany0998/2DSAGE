#ifndef PROJECTILEFIREDEVENT_H
#define PROJECTILEFIREDEVENT_H

#include "../EventBus/Event.h"
#include <glm/glm.hpp>
#include <string>

class ProjectileFiredEvent : public Event {

	public:
		std::string soundId;
		glm::vec2 position;
		ProjectileFiredEvent(glm::vec2 position, std::string soundId) {
			this->position = position;
			this->soundId = soundId;
		}
};


#endif //!PROJECTILEFIREDEVENT_H
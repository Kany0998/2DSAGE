#ifndef PROJECTILEFIREDEVENT_H
#define PROJECTILEFIREDEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"
#include <glm/glm.hpp>
#include <string>

class ProjectileFiredEvent : public Event {

	public:
		std::string soundId;
		glm::vec2 position;
		Entity shooter;
		glm::vec2 direction;
		int shotIntervalMs;
		ProjectileFiredEvent(Entity shooter, glm::vec2 position, glm::vec2 direction, std::string soundId, int shotIntervalMs) {
			this->shooter = shooter;
			this->position = position;
			this->direction = direction;
			this->soundId = soundId;
			this->shotIntervalMs = shotIntervalMs;
		}
};


#endif //!PROJECTILEFIREDEVENT_H
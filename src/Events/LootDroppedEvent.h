#ifndef LOOTDROPPEDEVENT_H
#define LOOTDROPPEDEVENT_H

#include "../EventBus/Event.h"
#include <glm/glm.hpp>
#include <string>


//Emitted when a loot bag appears; the banner and later the audio system react to it.
class LootDroppedEvent : public Event
{
	public:

		glm::vec2 bagPosition;
		std::string tierId;

		LootDroppedEvent(glm::vec2 bagPosition, std::string tierId)
			: bagPosition(bagPosition), tierId(tierId) {
		}
};


#endif // !LOOTDROPPEDEVENT_H

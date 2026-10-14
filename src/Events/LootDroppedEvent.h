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
		bool fromKill;

		LootDroppedEvent(glm::vec2 bagPosition, std::string tierId, bool fromKill)
			: bagPosition(bagPosition), tierId(tierId), fromKill(fromKill) {
		}
};


#endif // !LOOTDROPPEDEVENT_H

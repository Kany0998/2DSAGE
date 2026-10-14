#ifndef ITEMDROPPEDEVENT_H
#define ITEMDROPPEDEVENT_H

#include "../EventBus/Event.h"
#include <glm/glm.hpp>
#include <string>


//Emitted when the player drags an item out of the backpack onto the map; LootSystem puts it in a bag at that position.
class ItemDroppedEvent : public Event
{
public:

	glm::vec2 position;
	std::string itemId;

	ItemDroppedEvent(glm::vec2 position, std::string itemId)
		: position(position), itemId(itemId) {
	}
};


#endif //!ITEMDROPPEDEVENT_H
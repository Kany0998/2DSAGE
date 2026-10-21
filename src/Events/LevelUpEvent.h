#ifndef LEVELUPEVENT_H
#define LEVELUPEVENT_H


#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

class LevelUpEvent : public Event {

public:
	Entity player;
	int newLevel;

	LevelUpEvent(Entity player, int newLevel) {
		this->player = player;
		this->newLevel = newLevel;
	}
};

#endif // !LEVELUPEVENT_H

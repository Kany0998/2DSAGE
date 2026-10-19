#ifndef UISOUNDEVENT_H
#define UISOUNDEVENT_H

#include "../EventBus/Event.h"
#include <string>

class UISoundEvent : public Event {
	
	public:
		std::string action;

		UISoundEvent(std::string action) {
			this->action = action;
		}
};

#endif // !UISOUNDEVENT_H

#ifndef MOUSEBUTTONPRESSEDEVENT_H
#define MOUSEBUTTONPRESSEDEVENT_H

#include "../EventBus/Event.h"
#include <glm/glm.hpp>
#include <SDL.h>

class MouseButtonPressedEvent : public Event
{
	public:
		Uint8 button;
		glm::vec2 worldPosition; //already camera-adjusted, i.e. map/world space, not screen space

		MouseButtonPressedEvent(Uint8 button, glm::vec2 worldPosition) : button(button), worldPosition(worldPosition) {}
};

#endif

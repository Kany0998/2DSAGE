#ifndef KEYPRESSEDEVENT_H
#define KEYPRESSEDEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"
#include <SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Keycode symbol;
	bool isRepeat;   //true while a held key auto-repeats; toggles ignore those
	KeyPressedEvent(SDL_Keycode symbol, bool isRepeat = false) : symbol(symbol), isRepeat(isRepeat) {}
};

#endif
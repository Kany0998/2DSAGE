#ifndef ABILITYSTATECOMPONENT_H
#define ABILITYSTATECOMPONENT_H

#include <SDL.h>

struct AbilityStateComponent {
	Uint32 lastCastTime = 0;		//Take last succesful cast time form SDL_GetTicks();
};

#endif // !ABILITYSTATECOMPONENT_H

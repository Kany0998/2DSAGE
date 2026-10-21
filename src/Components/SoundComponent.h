#ifndef SOUNDCOMPONENT_H
#define SOUNDCOMPONENT_H

#include <string>
#include <SDL.h>

struct SoundComponent {
	//sound names for this entity's events; empty = silent
	std::string hurt;
	std::string death;
	std::string levelUp;
	std::string cast;
	std::string idle;
	Uint32 idleMinMs = 0;	//shortest gap between idle sounds
	Uint32 idleMaxMs = 0;	//longest gap
	Uint32 nextIdleAt = 0;	//state: when the next idle sound plays (SDL ticks), 0 = not drawn yet
	std::string loop;
	bool footsteps = false;
	int footstepChannel = -1;	//channel playing this entity's walking loop, -1 = none
	std::string footstepSound;	//which walking loop is playing right now "" = none
	int loopChannel = -1;
};


#endif // !SOUNDCOMPONENT_H

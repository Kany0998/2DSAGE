#ifndef LEVELAMBIENCE_H
#define LEVELAMBIENCE_H

#include <string>
#include <vector>
#include <sol/sol.hpp>
#include <SDL.h>

struct AmbientSound {
	std::string soundId;
	Uint32 minMs = 0;
	Uint32 maxMs = 0;
	Uint32 nextAt = 0;		//when it plays next (SDL ticks); set when the ambience starts
};

struct LevelAmbience {
	std::string loop;						//continuous background, "" = none
	std::vector<AmbientSound> random;		//sounds that come now and then
	static LevelAmbience Load(sol::state& lua);
};	


#endif // !LEVELAMBIENCE_H

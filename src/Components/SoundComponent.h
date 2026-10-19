#ifndef SOUNDCOMPONENT_H
#define SOUNDCOMPONENT_H

#include <string>

struct SoundComponent {
	//sound names for this entity's events; empty = silent
	std::string hurt;
	std::string death;
	std::string levelUp;
	std::string cast;
	std::string idle;
	std::string loop;
	bool footsteps = false;

	SoundComponent(std::string hurt = "", std::string death = "", std::string levelUp = "",
		std::string cast = "", std::string idle = "", std::string loop = "", bool footsteps = false) {
		this->hurt = hurt;
		this->death = death;
		this->levelUp = levelUp;
		this->cast = cast;
		this->idle = idle;
		this->loop = loop;
		this->footsteps = footsteps;
	}
};


#endif // !SOUNDCOMPONENT_H

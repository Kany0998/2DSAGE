#ifndef LOOTBAGCOMPONENT_H
#define LOOTBAGCOMPONENT_H

#include <string>
#include <vector>
#include <SDL.h>

struct LootBagComponent {
	std::string tierId;					//tier of the bag
	std::vector<std::string> items;		//packed list: taking an item removes it
	Uint32 spawnTime;					//moment of spawn of bag needed for destruction
	int lifetimeMs;						//how long bag stays on the ground

	LootBagComponent(std::string tierId = "", std::vector<std::string> items = {}, int lifetimeMs = 0) {
		this->tierId = tierId;
		this->items = items;
		this->spawnTime = SDL_GetTicks();
		this->lifetimeMs = lifetimeMs;
	}
};

#endif // !LOOTBAGCOMPONENT_H

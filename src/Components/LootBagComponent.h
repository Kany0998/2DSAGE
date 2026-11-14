#ifndef LOOTBAGCOMPONENT_H
#define LOOTBAGCOMPONENT_H

#include <string>
#include <vector>

struct LootBagComponent {
	std::string tierId;					//tier of the bag
	std::vector<std::string> items;		//packed list: taking an item removes it
	double ageMs = 0.0;					// grows only while LootSystem::Update runs so pause freezes it
	int lifetimeMs;						//how long bag stays on the ground

	LootBagComponent(std::string tierId = "", std::vector<std::string> items = {}, int lifetimeMs = 0) {
		this->tierId = tierId;
		this->items = items;
		this->lifetimeMs = lifetimeMs;
	}
};

#endif // !LOOTBAGCOMPONENT_H

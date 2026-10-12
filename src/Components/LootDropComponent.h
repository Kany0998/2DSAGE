#ifndef LOOTDROPCOMPONENT_H
#define LOOTDROPCOMPONENT_H

#include <string>

struct LootDropComponent {
	std::string tableId;		//name of a table in LootTables (loot.lua), the drops are rolled from it on death

	LootDropComponent(std::string tableId = "") {
		this->tableId = tableId;
	}
};

#endif // !LOOTDROPCOMPONENT_H

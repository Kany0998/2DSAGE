#ifndef ABILITYDATABASE_H
#define ABILITYDATABASE_H

#include "AbilityDefinition.h"
#include "../Inventory/ItemDatabase.h"
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

class AbilityDatabase {
	public:
		void Load(sol::state& lua, const std::string& path, const ItemDatabase& itemDatabase);
		const AbilityDefinition* Find(const std::string& id) const;
		const sol::table* StatsForItem(const std::string& itemId) const;    //nullptr if the item casts nothing

	private:
		std::unordered_map<std::string, AbilityDefinition> abilities;
		std::unordered_map<std::string, sol::table> itemStats;				//item id -> its ability_stats, with the ability's defaults behind it
};


#endif // !ABILITYDATABASE_H

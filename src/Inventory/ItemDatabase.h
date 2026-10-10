#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include "ItemDefinition.h"
#include <sol/sol.hpp>
#include <unordered_map>

class ItemDatabase {

	public:

		void Load(sol::state& lua, const std::string& path);
		const ItemDefinition* Find(const std::string& id)const;
		const std::unordered_map<std::string, ItemDefinition>& All() const { return items; }

	private:
		std::unordered_map<std::string, ItemDefinition> items;
};


#endif //! ITEMDATABASE_H
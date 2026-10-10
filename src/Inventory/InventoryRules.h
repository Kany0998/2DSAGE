#ifndef INVENTORYRULES_H
#define INVENTORYRULES_H

#include "../Components/AttributesComponent.h"
#include "../Components/EquipmentComponent.h"
#include "../Components/BackpackComponent.h"
#include "ItemDatabase.h"
#include <string>

class InventoryRules {

	public:

		static void RebuildBonuses(AttributesComponent& attributes, const EquipmentComponent& equipment, const ItemDatabase& items);

		static bool Equip(int backpackIndex, BackpackComponent& backpack, EquipmentComponent& equipment,
			AttributesComponent& attributes, const ItemDatabase& items, std::string& outMessage);

		static bool Unequip(int slotIndex, BackpackComponent& backpack, EquipmentComponent& equipment,
			AttributesComponent& attributes, const ItemDatabase& items, std::string& outMessage);

	private:
		static int FindSlotFor(const EquipmentComponent& equipment, const std::string& tag);
};



#endif // !INVENTORYRULES_H

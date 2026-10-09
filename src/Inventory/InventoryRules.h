#ifndef INVENTORYRULES_H
#define INVENTORYRULES_H

#include "../Components/AttributesComponent.h"
#include "../Components/EquipmentComponent.h"
#include "ItemDatabase.h"

class InventoryRules {

	public:

		static void RebuildBonuses(AttributesComponent& attributes, const EquipmentComponent& equipment, const ItemDatabase& items);

	private:

};



#endif // !INVENTORYRULES_H

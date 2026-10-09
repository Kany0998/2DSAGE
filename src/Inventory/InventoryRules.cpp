#include "InventoryRules.h"

void InventoryRules::RebuildBonuses(AttributesComponent& attributes, const EquipmentComponent& equipment, const ItemDatabase& items)
{
    for (int i = 0; i < Skill_Count; i++) {
        attributes.bonus[i] = 0;
    }
    
    for (const auto& slot : equipment.slots) {
        if (slot.itemId.empty()) {
            continue;
        }

        const ItemDefinition* definition = items.Find(slot.itemId);
        if (definition == nullptr) {
            continue;
        }

        for (int i = 0; i < Skill_Count; i++) {
            attributes.bonus[i] += definition->bonus[i];
        }
    }


}
#include "InventoryRules.h"
#include "../Logger/Logger.h"

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



bool InventoryRules::Equip(int backpackIndex, BackpackComponent& backpack, EquipmentComponent& equipment,
    AttributesComponent& attributes, const ItemDatabase& items, std::string& outMessage) {
    
    if (backpackIndex < 0 || backpackIndex >= static_cast<int>(backpack.items.size())) {
        outMessage = "No such item";
        return false;
    }

    if (backpack.items[backpackIndex].empty()) {
        outMessage = "No such item";
        return false;
    }

    const std::string itemId = backpack.items[backpackIndex];
    const ItemDefinition* definition = items.Find(itemId);
    if (definition == nullptr) {
        outMessage = "Unknown item";
        return false;
    }

    int slotIndex = FindSlotFor(equipment, definition->tag);
    if (slotIndex == -1) {
        outMessage = "Nothing can hold this";
        return false;
    }

    std::string oldId = equipment.slots[slotIndex].itemId;
    equipment.slots[slotIndex].itemId = itemId;

    if (oldId.empty()) {
        //slot was free: the item leaves its square, which stays empty
        backpack.items[backpackIndex].clear();
    }
    else {
        // swap: the old item takes the new one's place in the backpack
        backpack.items[backpackIndex] = oldId;
    }
    RebuildBonuses(attributes, equipment, items);
    return true;
}


bool InventoryRules::Unequip(int slotIndex, BackpackComponent& backpack, EquipmentComponent& equipment,
    AttributesComponent& attributes, const ItemDatabase& items, std::string& outMessage) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(equipment.slots.size())) {
        outMessage = "No such slot";
        return false;
    }

    if (equipment.slots[slotIndex].itemId.empty()) {
        outMessage = "Nothing equipped there";
        return false;
    }

    if (backpack.IsFull()) {
        outMessage = "Backpack is full";
        return false;
    }

    backpack.Add(equipment.slots[slotIndex].itemId);    //into the first free square
    equipment.slots[slotIndex].itemId.clear();

    RebuildBonuses(attributes, equipment, items);
    return true;
}



int InventoryRules::FindSlotFor(const EquipmentComponent& equipment, const std::string& tag) {
    for (int i = 0; i < static_cast<int>(equipment.slots.size()); i++) {
        if (equipment.slots[i].accepts == tag && equipment.slots[i].itemId.empty()) {
            return i;
        }
    }

    for (int i = 0; i < static_cast<int>(equipment.slots.size()); i++) {
        if (equipment.slots[i].accepts == tag) {
            return i;
        }
    }
    return -1;
}
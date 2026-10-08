#ifndef EQUIPMENTCOMPONENT_H
#define EQUIPMENTCOMPONENT_H

#include <string>
#include <vector>

struct EquipmentSlot {
	std::string name;		//"Weapon", "Helm"... shown in the I window
	std::string accepts;	//the one tag this slot takes
	std::string itemId;		//empty = nothing equipped
};

struct EquipmentComponent {
	std::vector<EquipmentSlot> slots;	//in Lua order = order in the I window

	int FindSlot(const std::string& name) const {
		for (int i = 0; i < static_cast<int>(slots.size()); i++) {
			if (slots[i].name == name) {
				return i;
			}
		}
		return -1;
	}
};

#endif // !EQUIPMENTCOMPONENT_H

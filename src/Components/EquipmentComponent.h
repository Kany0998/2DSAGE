#ifndef EQUIPMENTCOMPONENT_H
#define EQUIPMENTCOMPONENT_H

#include <string>
#include <vector>

struct EquipmentSlot {
	std::string name;		//"Weapon", "Helm"... shown in the I window
	std::string accepts;	//the one tag this slot takes
	std::string itemId;		//empty = nothing equipped
	int column = -1;		//grid cell in the equipment window, from Lua; the loader places
	int row = -1;			//slots without one in a row below the others (-1 only before loading)
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

	//index of the slot placed on this grid cell, or -1 if the cell is free
	int FindSlotAt(int column, int row) const {
		for (int i = 0; i < static_cast<int>(slots.size()); i++) {
			if (slots[i].column == column && slots[i].row == row) {
				return i;
			}
		}
		return -1;
	}
};

#endif // !EQUIPMENTCOMPONENT_H

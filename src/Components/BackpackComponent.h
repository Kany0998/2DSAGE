#ifndef BACKPACKCOMPONENT_H
#define BACKPACKCOMPONENT_H

#include <vector>
#include <string>

struct BackpackComponent {
	std::vector<std::string> items;		//one entry per square, in grid order; "" = free square
	int capacity = 0;					//0 or less = unlimited

	bool IsFull() const {
		return capacity > 0 && Count() >= capacity;
	}

	//how many squares hold an item - the "8" in "8 / 30"
	int Count() const {
		int count = 0;
		for (const auto& id : items) {
			if (!id.empty()) {
				count++;
			}
		}
		return count;
	}

	//Puts an item into the first free square; returns false when the backpack is full.
	//Free squares are reused first, so an item fills the gap another one left.
	bool Add(const std::string& id) {
		for (auto& square : items) {
			if (square.empty()) {
				square = id;          //reuse the first free square
				return true;
			}
		}

		//no free square: open a new one at the end, unless the limit is reached
		if (capacity > 0 && static_cast<int>(items.size()) >= capacity) {
			return false;
		}
		items.push_back(id);
		return true;
	}
};

#endif // !BACKPACKCOMPONENT_H

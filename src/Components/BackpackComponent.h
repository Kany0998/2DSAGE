#ifndef BACKPACKCOMPONENT_H
#define BACKPACKCOMPONENT_H

#include <vector>
#include <string>
#include <algorithm>

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

	//Moves the item in square "from" to square "to"; an item already in "to" goes to "from" (swap).
	//Returns false when nothing could be moved.
	bool MoveItem(int from, int to) {
		if (from == to) {
			return false;
		}

		//from must be an existing square
		if (from < 0 || from >= static_cast<int>(items.size())) {
			return false;
		}

		//From is empty
		if (items[from].empty()) {
			return false;
		}

		//to cannot be negative
		if (to < 0) {
			return false;
		}

		//a limited backpack has no squares beyond its capacity
		if (capacity > 0 && to >= capacity) {
			return false;
		}

		//the empty squares at the end may not exist in the vector yet: create them ("" = free)
		if (to >= static_cast<int>(items.size())) {
			items.resize(to + 1);
		}

		std::swap(items[from], items[to]);
		return true;
	}
};

#endif // !BACKPACKCOMPONENT_H

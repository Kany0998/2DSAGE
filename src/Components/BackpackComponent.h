#ifndef BACKPACKCOMPONENT_H
#define BACKPACKCOMPONENT_H

#include <vector>
#include <string>

struct BackpackComponent {
	std::vector<std::string> items;		//item ids, in the order they were added
	int capacity = 0;					//0 or less = unlimited

	bool IsFull() const {
		return capacity > 0 && static_cast<int>(items.size()) >= capacity;
	}
};

#endif // !BACKPACKCOMPONENT_H

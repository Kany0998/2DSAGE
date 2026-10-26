#ifndef PATHPENALTY_H
#define PATHPENALTY_H

#include <vector>


//call Clear(cellCount) before Add/At
struct PathPenalty {
	
	void Clear(int cellCount) {
		if (stampPerCell.size() != static_cast<size_t>(cellCount)) {
			extraCostPerCell.assign(cellCount, 0.0);
			stampPerCell.assign(cellCount, 0);
		}
		currentId++;

		if (currentId == 0) {
			stampPerCell.assign(cellCount, 0);
			currentId = 1;
		}
	}


	void Add(int index, double value) {
		if (stampPerCell[index] != currentId) {
			stampPerCell[index] = currentId;
			extraCostPerCell[index] = 0;
		}
		extraCostPerCell[index] += value;
	}

	double ExtraCostAt(int index)const {
		if (stampPerCell[index] != currentId) {
			return 0.0;
		}
		return extraCostPerCell[index];
	}


	private:
		std::vector<double> extraCostPerCell;
		std::vector<unsigned int> stampPerCell;
		unsigned int currentId = 0;
};

#endif // !PATHPENALTY_H

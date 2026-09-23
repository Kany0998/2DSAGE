#include "Pathfinder.h"
#include "../TileMap/TileMap.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <queue>
#include <functional>

double Pathfinder::OctileHeuristic(int col, int row, int goalCol, int goalRow) const {
	int dx = std::abs(goalCol - col);
	int dy = std::abs(goalRow - row);

	return (dx + dy) + (diagonalCost - 2.0) * std::min(dx, dy);
}



double Pathfinder::CostToEnter(bool isDiagonal) const{

	if (isDiagonal) {
		return diagonalCost;
	}

	return 1.0;
}



void Pathfinder::FindNeighbours(const TileMap& tilemap, int currentCellIndex, int movementType, std::vector<std::pair<int, double>>& outNeighbours) const {

	outNeighbours.clear();

	int col = tilemap.IndexToCol(currentCellIndex);
	int row = tilemap.IndexToRow(currentCellIndex);

	for (int i = 0; i < directionCount; ++i){
		
		int dx = dirX[i];
		int dy = dirY[i];

		int neighbourCol = col + dx;
		int neighbourRow = row + dy;

		if (tilemap.isBlocked(neighbourCol, neighbourRow, movementType)) {
			continue;
		}

		bool isDiagonal = (dx != 0 && dy != 0);

		if (isDiagonal) {
			if (tilemap.isBlocked(col + dx, row, movementType) || tilemap.isBlocked(col, row + dy, movementType)) {
				continue;
			}
		}

		int neighbourIndex = tilemap.Index(neighbourCol, neighbourRow);
		double cost = CostToEnter(isDiagonal);

		outNeighbours.push_back({ neighbourIndex, cost });
	}
}



bool Pathfinder::FindPath(const TileMap& tilemap, int startCol, int startRow, int goalCol, int goalRow, int movementType, std::vector<int>& outPath) {
	
	int cols = tilemap.Cols();
	int rows = tilemap.Rows();

	int cellCount = cols * rows;

	costSoFar.assign(cellCount, -1.0); //never reached
	cameFrom.assign(cellCount, -1);
	tilesState.assign(cellCount, unreachedTile);//unreached

	outPath.clear();

	if (!tilemap.isInside(startCol, startRow) || !tilemap.isInside(goalCol, goalRow)) {
		return false;
	}

	if (tilemap.isBlocked(goalCol, goalRow, movementType)) {
		return false;
	}

	int startIndex = tilemap.Index(startCol, startRow);
	int goalIndex = tilemap.Index(goalCol, goalRow);

	if (startIndex == goalIndex) {
		outPath.push_back(goalIndex);
		return true;
	}

	std::priority_queue<
		std::pair<double, int>,
		std::vector<std::pair<double, int>>,
		std::greater<std::pair<double, int>>
	> frontierQueue;

	costSoFar[startIndex] = 0.0;
	tilesState[startIndex] = frontierTile;
	frontierQueue.push({0.0, startIndex});

	std::vector<std::pair<int, double>> neighbours;

	while (!frontierQueue.empty()) {

		int current = frontierQueue.top().second;
		frontierQueue.pop();

		if (tilesState[current] == exploredTile) {
			continue;
		}

		tilesState[current] = exploredTile;

		if (current == goalIndex) {
			break;
		}



		FindNeighbours(tilemap, current, movementType, neighbours);

		for (const auto& nb : neighbours) {
			int next = nb.first;
			double stepCost = nb.second;

			double newCost = costSoFar[current] + stepCost;

			if (costSoFar[next] < 0.0 || newCost < costSoFar[next]) {

				costSoFar[next] = newCost;
				cameFrom[next] = current;

				if (tilesState[next] == unreachedTile) {
					tilesState[next] = frontierTile;
				}

				double estimatedCostToGoal = OctileHeuristic(tilemap.IndexToCol(next), tilemap.IndexToRow(next), goalCol, goalRow);

				frontierQueue.push({ newCost + estimatedCostToGoal, next });
			}
		}

	}

	if (costSoFar[goalIndex] < 0.0) {
		return false;
	}

	int cell = goalIndex;

	while (cell != startIndex) {
		outPath.push_back(cell);
		cell = cameFrom[cell];
	}
	std::reverse(outPath.begin(), outPath.end());

	if (smoothingEnable) {
		SmoothPath(tilemap, startIndex, movementType, outPath);
	}

	return true;
}

bool Pathfinder::HasLineOfSight(const TileMap& tilemap, int fromIndex, int toIndex, int movementType) const {
	double tileWorldSize = tilemap.TileWorldSize();
	double fromX = tilemap.IndexToCol(fromIndex) * tileWorldSize + tileWorldSize / 2.0;
	double fromY = tilemap.IndexToRow(fromIndex) * tileWorldSize + tileWorldSize / 2.0;
	double toX = tilemap.IndexToCol(toIndex) * tileWorldSize + tileWorldSize / 2.0;
	double toY = tilemap.IndexToRow(toIndex) * tileWorldSize + tileWorldSize / 2.0;

	const double dx = toX - fromX;
	const double dy = toY - fromY;
	
	//Sample at least twice per tile, so no blocked tile can hide between two samples
	const int steps = static_cast<int>(std::ceil(std::max(std::abs(dx), std::abs(dy)) / (tileWorldSize / 2.0)));
	
	if (steps < 1) {
		return true;
	}

	for (int i = 1; i <= steps; ++i) {
		const double t = static_cast<double>(i) / steps;
		const double x = fromX + dx * t;
		const double y = fromY + dy * t;

		if (tilemap.isBlockedAtWorld(x, y, movementType)) {
			return false;
		}
	}
	return true;
}

void Pathfinder::SmoothPath(const TileMap& tilemap, int startIndex, int movementType, std::vector<int>& path) const {
	if (path.size() < 2) {
		return;
	}

	const int cellCount = static_cast<int>(path.size());
	std::vector<int> smoothed;
	smoothed.reserve(path.size());

	int anchor = startIndex;	//Where the entity actually stands; the first straight line starts here
	int i = 0;

	while (i < cellCount) {

		//Furthest cell on the remaining route that can be reached in a straight line
		int best = i;
		for (int j = cellCount - 1; j > i; --j) {
			if (HasLineOfSight(tilemap, anchor, path[j], movementType)) {
				best = j;
				break;
			}
		}
		smoothed.push_back(path[best]);
		anchor = path[best];
		i = best + 1;
	}
	path.swap(smoothed);
}



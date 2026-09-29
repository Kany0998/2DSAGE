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



double Pathfinder::CostToEnter(const TileMap& tilemap, int col, int row, bool isDiagonal) const {

	double speedMultiplier = tilemap.SpeedMultiplierAt(col, row);
	if (speedMultiplier <= 0) {
		speedMultiplier = 0.0000001;	//Some field can have multiplier as 0 so player will be temporary blocked there but we try to skip them for AI
	}

	if (isDiagonal) {
		return diagonalCost / speedMultiplier;
	}

	return stepCost / speedMultiplier;
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

			//A diagonal grazes the corner shared with both orthogonal cells, so the entity
			//really does travel through them: charge for the worst of the three, or the
			//search gets a corner crossing for free
			const double sideA = tilemap.SpeedMultiplierAt(col + dx, row);
			const double sideB = tilemap.SpeedMultiplierAt(col, row + dy);

			if (sideA < diagonalMinMultiplier || sideB < diagonalMinMultiplier) {
				continue;
			}
		}

		int neighbourIndex = tilemap.Index(neighbourCol, neighbourRow);
		double cost = CostToEnter(tilemap, neighbourCol, neighbourRow, isDiagonal);

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

double Pathfinder::LineCost(const TileMap& tilemap, int fromIndex, int toIndex, int movementType, double minMultiplier) const {

	const double tileWorldSize = tilemap.TileWorldSize();

	const double fromX = tilemap.IndexToCol(fromIndex) * tileWorldSize + tileWorldSize / 2.0;
	const double fromY = tilemap.IndexToRow(fromIndex) * tileWorldSize + tileWorldSize / 2.0;
	const double toX = tilemap.IndexToCol(toIndex) * tileWorldSize + tileWorldSize / 2.0;
	const double toY = tilemap.IndexToRow(toIndex) * tileWorldSize + tileWorldSize / 2.0;

	const double dx = toX - fromX;
	const double dy = toY - fromY;

	const int steps = static_cast<int>(std::ceil(std::max(std::abs(dx), std::abs(dy)) / (tileWorldSize / lineSamplesPerTile)));

	if (steps < 1) {
		return 0.0;
	}

	const double lineLength = std::sqrt(dx * dx + dy * dy);
	const double distancePerSample = lineLength / steps;

	//An entity does not follow this line exactly: it starts anywhere in its own cell and
	//counts a waypoint as reached within the arrival radius. The line is therefore tested
	//as a corridor that wide, not as a line
	const double offsetX = -dy / lineLength * tileWorldSize * lineClearanceInTiles;
	const double offsetY = dx / lineLength * tileWorldSize * lineClearanceInTiles;

	double cost = 0.0;

	for (int i = 1; i <= steps; ++i) {
		const double t = static_cast<double>(i) / steps;
		const double x = fromX + dx * t;
		const double y = fromY + dy * t;

		double worstMultiplier = 1.0;

		for (int probe = -1; probe <= 1; ++probe) {
			const double probeX = x + offsetX * probe;
			const double probeY = y + offsetY * probe;

			if (tilemap.isBlockedAtWorld(probeX, probeY, movementType)) {
				return -1.0;
			}

			double multiplier = tilemap.SpeedMultiplierAtWorld(probeX, probeY);

			if (multiplier <= 0.0) {
				multiplier = 0.0000001;
			}

			if (multiplier < worstMultiplier) {
				worstMultiplier = multiplier;
			}
		}

		//Never straighten across ground the search went out of its way to avoid
		if (worstMultiplier < minMultiplier) {
			return -1.0;
		}

		cost += (distancePerSample / tileWorldSize) / worstMultiplier;
	}

	return cost;
}

double Pathfinder::PathSegmentCost(const TileMap& tilemap, int fromIndex, const std::vector<int>& path, int firstStep, int lastStep) const {

	double cost = 0.0;
	int previous = fromIndex;

	for (int k = firstStep; k <= lastStep; ++k) {
		const int next = path[k];

		const int previousCol = tilemap.IndexToCol(previous);
		const int previousRow = tilemap.IndexToRow(previous);
		const int nextCol = tilemap.IndexToCol(next);
		const int nextRow = tilemap.IndexToRow(next);

		const bool isDiagonal = (nextCol != previousCol) && (nextRow != previousRow);

		cost += CostToEnter(tilemap, nextCol, nextRow, isDiagonal);
		previous = next;
	}
	return cost;
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
			const double lineCost = LineCost(tilemap, anchor, path[j], movementType, smoothingMinMultiplier);

			if(lineCost < 0.0){
				continue;
			}

			const double segmentCost = PathSegmentCost(tilemap, anchor, path, i, j);

			//Straighten only when the shortcut is no dearer than the route it replaces,
			//so a detour the search made around slow ground survives smoothing
			if (lineCost <= segmentCost + costEpsilon) {
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



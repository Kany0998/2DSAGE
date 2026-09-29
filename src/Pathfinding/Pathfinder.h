#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <utility>
#include <vector>

class TileMap;

class Pathfinder {

	private:
		std::vector<double> costSoFar;
		std::vector<int> cameFrom;
		std::vector<int> tilesState;
		static constexpr double diagonalCost = 1.41421356237;

		

		//Tile information
		static constexpr int unreachedTile = 0; //Search hasnt find this file yet
		static constexpr int frontierTile = 1;	//The search knows it exists and has a cost for it, but hasn't checked its neighbours yet
		static constexpr int exploredTile = 2;	//the search has taken it out of the queue and checked its neighbours
		static constexpr double costEpsilon = 1e-6;

		//8-way movement
		static constexpr int directionCount = 8;
		static constexpr int dirX[directionCount] = { 0,  1, 1, 1, 0, -1, -1, -1};
		static constexpr int dirY[directionCount] = {-1, -1, 0, 1, 1,  1,  0, -1};
												    //up, ur, rg, dr, dw, dl, lf, ul
		static constexpr double stepCost = 1.0;

		//How far to either side of a line each sample is probed, so a shortcut cannot
		//clip the corner of a tile unnoticed
		double lineClearanceInTiles = 0.25;

		//A shortcut may not cross ground slower than this, whatever the arithmetic says
		double smoothingMinMultiplier = 0.5;

		//A diagonal may not cut the corner of ground slower than this
		static constexpr double diagonalMinMultiplier = 0.2;

		double OctileHeuristic(int col, int row, int goalCol, int goalRow) const;
		void FindNeighbours(const TileMap& tilemap, int currentCellIndex, int movementType, std::vector<std::pair<int, double>>& outNeighbours) const;
		double CostToEnter(const TileMap& tilemap, int col, int row, bool isDiagonal) const;

		double LineCost(const TileMap& tilemap, int fromIndex, int toIndex, int movementType, double minMultiplier) const;
		void SmoothPath(const TileMap& tilemap, int startIndex, int movementType, std::vector<int>& path) const;
		double PathSegmentCost(const TileMap& tilemap, int fromIndex, const std::vector<int>& path, int firstStep, int lastStep) const;

	public:
		bool smoothingEnable = true;
		//Line samples per tile: higher is more accurate about thin obstacles and narrow
		//bands of slow ground, at a linear cost in lookups
		double lineSamplesPerTile = 4.0;
		bool FindPath(const TileMap& tilemap, int startCol, int startRow, int goalCol, int goalRow,int movementType, std::vector<int>& outPath);
};


#endif

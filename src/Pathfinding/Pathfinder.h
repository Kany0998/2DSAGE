#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "PathPenalty.h"
#include <utility>
#include <vector>
#include <limits>

class TileMap;


struct SearchStats {
	//Data for measurements
	int nodesExpanded = 0;
	int nodesPushed = 0;
	double pathCost = -1; // -1 means no path 0 is real cost start == goal
	bool found = false;
	double resetMs = 0.0;
	double searchMs = 0.0;
	double smoothMs = 0.0;
};

struct SearchBounds {
	int minCol = 0;
	int minRow = 0;
	int maxCol = std::numeric_limits<int>::max();
	int maxRow = std::numeric_limits<int>::max();
	bool Contains(int col, int row)  const{
		return col >= minCol && col <= maxCol && row >= minRow && row <= maxRow;
	}
};


class Pathfinder {

	private:
		std::vector<double> costSoFar;
		std::vector<int> cameFrom;
		std::vector<int> tilesState;
		std::vector<unsigned int> stamp;
		unsigned int searchId = 0;
		SearchStats lastStats;		//measurements only
		static constexpr double diagonalCost = 1.41421356237;

		
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
		void FindNeighbours(const TileMap& tilemap, int currentCellIndex, int movementType, std::vector<std::pair<int, double>>& outNeighbours, const SearchBounds& bounds, const PathPenalty* pathPenalty) const;
		double CostToEnter(const TileMap& tilemap, int col, int row, bool isDiagonal) const;

		double LineCost(const TileMap& tilemap, int fromIndex, int toIndex, int movementType, double minMultiplier, const PathPenalty* pathPenalty) const;
		void SmoothPath(const TileMap& tilemap, int startIndex, int movementType, std::vector<int>& path, const PathPenalty* pathPenalty) const;
		double PathSegmentCost(const TileMap& tilemap, int fromIndex, const std::vector<int>& path, int firstStep, int lastStep, const PathPenalty* pathPenalty) const;

		void Touch(int index) {
			if (stamp[index] == searchId) {
				return;
			}

			stamp[index] = searchId;
			costSoFar[index] = -1.0;
			cameFrom[index] = -1;
			tilesState[index] = unreachedTile;
		}

	public:
		bool smoothingEnable = true;
		//Line samples per tile: higher is more accurate about thin obstacles and narrow
		//bands of slow ground, at a linear cost in lookups
		double lineSamplesPerTile = 4.0;
		bool FindPath(const TileMap& tilemap, int startCol, int startRow, int goalCol, int goalRow, int movementType,
			std::vector<int>& outPath, const SearchBounds& bounds = SearchBounds{}, const PathPenalty* pathPenalty = nullptr);

		const SearchStats& LastStats() const {
			return lastStats;
		}

		int TileStateAt(int index) const {
			if (stamp[index] != searchId) {
				return unreachedTile;
			}
			else {
				return tilesState[index];
			}
		}



		//queue priority = gWeight * g + hWeight * h: 1/0 Dijkstra, 1/1 A*, 0/1 greedy (path cost always stays real)
		double gWeight = 1.0;
		double hWeight = 1.0;

		//Tile information
		static constexpr int unreachedTile = 0; //Search hasnt find this tile yet
		static constexpr int frontierTile = 1;	//The search knows it exists and has a cost for it, but hasn't checked its neighbours yet
		static constexpr int exploredTile = 2;	//the search has taken it out of the queue and checked its neighbours
};


#endif

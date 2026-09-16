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

		//8-way movement
		static constexpr int directionCount = 8;
		static constexpr int dirX[directionCount] = { 0,  1, 1, 1, 0, -1, -1, -1};
		static constexpr int dirY[directionCount] = {-1, -1, 0, 1, 1,  1,  0, -1};
												    //up, ur, rg, dr, dw, dl, lf, ul

		double OctileHeuristic(int col, int row, int goalCol, int goalRow) const;
		void FindNeighbours(const TileMap& tilemap, int currentCellIndex, int movementType, std::vector<std::pair<int, double>>& outNeighbours) const;
		double CostToEnter(bool isDiagonal) const;

	public:
		
		bool FindPath(const TileMap& tilemap, int startCol, int startRow, int goalCol, int goalRow,int movementType, std::vector<int>& outPath);
};


#endif

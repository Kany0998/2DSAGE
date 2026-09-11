#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>

class TileMap {
	private:
		std::vector<int> tileIds;
		int rows = 0;
		int cols = 0;
		int tileSize = 0;
		double tileScale = 0.0;
		double tileWorldSize = 0.0;
		static constexpr int tileIdCount = 100;	

		// What each tile id blocks, indexed by the id itself. Ids in the .map file are
		// two digits, so 100 slots covers every one of them. 0 means "blocks nothing",
		// which is why Lua only lists obstacles. Bit values come from MovementType.h.
		int blockMasks[tileIdCount] = {};

		int Index(int col, int row) const;


	public:

		void load(const std::string& filePath, int mapRows, int mapCols, int mapTileSize, double mapTileScale);
		void setBlockMask(int tileId, int mask);

		int colAt(double worldX) const;
		int rowAt(double worldY) const;

		bool isInside(int col, int row) const;
		int tileAt(int col, int row) const;
		bool isBlocked(int col, int row, int movementType) const;
		bool isBlockedAtWorld(double worldX, double worldY, int movementType) const;

		int Cols() const { return cols; }
		int Rows() const { return rows; }
		double TileWorldSize() const { return tileWorldSize; }
};


#endif // !TILEMAP_H

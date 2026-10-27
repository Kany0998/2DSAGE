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
		std::string textureAssetId;
		double tileScale = 0.0;
		double tileWorldSize = 0.0;
		static constexpr int tileIdCount = 100;

		// What each tile id blocks, indexed by the id itself. Ids in the .map file are
		// two digits, so 100 slots covers every one of them. 0 means "blocks nothing",
		// which is why Lua only lists obstacles. Bit values come from MovementType.h.
		int blockMasks[tileIdCount] = {};
		double speedMultipliers[tileIdCount] = {};	//1.0 - normal gorund, less - for example water slower, more - for example road faster
		double frictions[tileIdCount] = {};
		double accelerations[tileIdCount] = {};
		std::string stepSounds[tileIdCount] = {};



	public:

		int Index(int col, int row) const;

		int IndexToRow(int index) const;
		int IndexToCol(int index) const;

		void load(const std::string& filePath, int mapRows, int mapCols, int mapTileSize, double mapTileScale);
		void setBlockMask(int tileId, int mask);
		void setSpeedMultiplier(int tileId, double speedMultiplier);
		void setFriction(int tileId, double friction);
		void setAcceleration(int tileId, double acceleration);
		void setTextureAssetId(const std::string& id);
		void setStepSound(int tileId, const std::string& id);

		int colAt(double worldX) const;
		int rowAt(double worldY) const;

		bool isInside(int col, int row) const;
		int tileAt(int col, int row) const;
		void SetTile(int col, int row, int tileId);
		double SpeedMultiplierAt(int col, int row) const;
		double MaxSpeedMultiplier() const;
		const std::string& StepSoundAt(int col, int row) const;
		bool isBlocked(int col, int row, int movementType) const;
		bool isBlockedAtWorld(double worldX, double worldY, int movementType) const;
		double SpeedMultiplierAtWorld(double worldX, double worldY) const;
		std::string StepSoundAtWorld(double worldX, double worldY) const;
		double FrictionAt(int col, int row) const;
		double AccelerationAt(int col, int row) const;

		int Cols() const { return cols; }
		int Rows() const { return rows; }
		double TileWorldSize() const { return tileWorldSize; }
		const std::string& TextureAssetId() const { return textureAssetId; }
		int TileSize() const { return tileSize; }
};


#endif // !TILEMAP_H

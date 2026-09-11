#include "TileMap.h"
#include "../Logger/Logger.h"
#include <fstream>
#include <cmath>
#include <string>



int TileMap::Index(int col, int row) const
{
	return row * cols + col;
}



bool TileMap::isInside(int col, int row) const
{
	return col >= 0 && col < cols && row >= 0 && row < rows;
}



int TileMap::tileAt(int col, int row) const
{
	if(!isInside(col, row))
	{
		return -1;
	}
	return tileIds[Index(col, row)];
}



void TileMap::setBlockMask(int tileId, int mask) {

	if(tileId >= 0 && tileId < tileIdCount)
	{
		blockMasks[tileId] = mask;
	}

	else
	{
		Logger::Err("Tile ID is out of range 0-" + std::to_string(tileIdCount - 1) + ": " + std::to_string(tileId));
	}
}



int TileMap::colAt(double worldX) const
{
	return static_cast<int>(std::floor(worldX / tileWorldSize));
}



int TileMap::rowAt(double worldY) const
{
	return static_cast<int>(std::floor(worldY / tileWorldSize));
}



bool TileMap::isBlocked(int col, int row, int movementType) const
{
	if (!isInside(col, row))
	{
		return true;
	}

	int tileId = tileAt(col, row);

	if(tileId < 0 || tileId >= tileIdCount)
	{
		return true;
	}

	return (blockMasks[tileId] & movementType) != 0;
}

bool TileMap::isBlockedAtWorld(double worldX, double worldY, int movementType) const
{
	int col = colAt(worldX);
	int row = rowAt(worldY);
	return isBlocked(col, row, movementType);
}

void TileMap::load(const std::string& filePath, int mapRows, int mapCols, int mapTileSize, double mapTileScale)
{
	rows = mapRows;
	cols = mapCols;
	tileSize = mapTileSize;
	tileScale = mapTileScale;

	tileWorldSize = tileSize * tileScale;

	tileIds.assign(rows * cols, 0);

	std::ifstream mapFile(filePath);

	if (!mapFile.is_open())
	{
		Logger::Err("Failed to open map file: " + filePath);
		return;
	}


	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			char firstDigit;
			char secondDigit;

			if (!mapFile.get(firstDigit) || !mapFile.get(secondDigit))
			{
				Logger::Err("Unexpected end of map file: " + filePath);
				return;
			}

			int tileId = (firstDigit - '0') * 10 + (secondDigit - '0');
			tileIds[Index(x, y)] = tileId;
			mapFile.ignore(); // Ignore the space or ,
		}
	}
}
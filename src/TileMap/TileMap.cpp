#include "TileMap.h"
#include "../Logger/Logger.h"
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <stdexcept>
#include <filesystem>



int TileMap::Index(int col, int row) const{
	return row * cols + col;
}

int TileMap::IndexToRow(int index) const{
	return index / cols;
}

int TileMap::IndexToCol(int index) const{
	return index % cols;
}

bool TileMap::isInside(int col, int row) const{
	return col >= 0 && col < cols && row >= 0 && row < rows;
}



int TileMap::tileAt(int col, int row) const{
	if(!isInside(col, row)){
		return -1;
	}
	return tileIds[Index(col, row)];
}

void TileMap::SetTile(int col, int row, int tileId) {
	if (!isInside(col, row)){
		return;
	}

	if (tileId < 0 || tileId >= tileIdCount) {
		return;
	}

	tileIds[Index(col, row)] = tileId;
}

double TileMap::SpeedMultiplierAt(int col, int row) const {

	if (!isInside(col, row)){
		return 1;
	}
	int tileId = tileIds[Index(col, row)];
	return speedMultipliers[tileId];
}

double TileMap::MaxSpeedMultiplier() const {
	double highest = 1.0;
	for (auto multiplier : speedMultipliers) {
		if (multiplier > highest) {
			highest = multiplier;
		}
	}
	return highest;
}

double TileMap::FrictionAt(int col, int row) const{
	if (!isInside(col, row))
	{
		return 1;
	}
	int tileId = tileIds[Index(col, row)];
	return frictions[tileId];
}


double TileMap::AccelerationAt(int col, int row) const{
	if (!isInside(col, row))
	{
		return 0.0;
	}
	int tileId = tileIds[Index(col, row)];
	return accelerations[tileId];
}

const std::string& TileMap::StepSoundAt(int col, int row) const {
	static const std::string empty;
	if (!isInside(col, row))
	{
		return empty;
	}
	int tileId = tileIds[Index(col, row)];
	return stepSounds[tileId];
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

void TileMap::setSpeedMultiplier(int tileId, double speedMultiplier) {

	if (tileId >= 0 && tileId < tileIdCount)
	{
		speedMultipliers[tileId] = speedMultiplier;
	}

	else
	{
		Logger::Err("Tile ID is out of range 0-" + std::to_string(tileIdCount - 1) + ": " + std::to_string(tileId) + " for speed mulitplier");
	}
}

void TileMap::setFriction(int tileId, double friction) {
	if (tileId >= 0 && tileId < tileIdCount){
		frictions[tileId] = friction;
	}

	else{
		Logger::Err("Tile ID is out of range 0-" + std::to_string(tileIdCount - 1) + ": " + std::to_string(tileId) + " for friction");
	}
}


void TileMap::setAcceleration(int tileId, double acceleration){
	if (tileId >= 0 && tileId < tileIdCount){
		accelerations[tileId] = acceleration;
	}

	else{
		Logger::Err("Tile ID is out of range 0-" + std::to_string(tileIdCount - 1) + ": " + std::to_string(tileId) + " for acceleration");
	}
}

void TileMap::setTextureAssetId(const std::string& id) {
	textureAssetId = id;
}

void TileMap::setStepSound(int tileId, const std::string& id) {
	if (tileId >= 0 && tileId < tileIdCount) {
		stepSounds[tileId] = id;
	}
	else {
		Logger::Warn("Tile ID is out of range 0-" + std::to_string(tileIdCount - 1) + ": " + std::to_string(tileId) + " for step sounds");
	}

}



int TileMap::colAt(double worldX) const{
	return static_cast<int>(std::floor(worldX / tileWorldSize));
}



int TileMap::rowAt(double worldY) const{
	return static_cast<int>(std::floor(worldY / tileWorldSize));
}



bool TileMap::isBlocked(int col, int row, int movementType) const{
	if (!isInside(col, row)){
		return true;
	}

	int tileId = tileAt(col, row);

	if(tileId < 0 || tileId >= tileIdCount){
		return true;
	}

	return (blockMasks[tileId] & movementType) != 0;
}

bool TileMap::isBlockedAtWorld(double worldX, double worldY, int movementType) const{
	int col = colAt(worldX);
	int row = rowAt(worldY);
	return isBlocked(col, row, movementType);
}

double TileMap::SpeedMultiplierAtWorld(double worldX, double worldY) const {
	int col = colAt(worldX);
	int row = rowAt(worldY);
	return SpeedMultiplierAt(col, row);
}

std::string TileMap::StepSoundAtWorld(double worldX, double worldY) const {
	int col = colAt(worldX);
	int row = rowAt(worldY);
	return StepSoundAt(col, row);
}

void TileMap::load(const std::string& filePath, int mapTileSize, double mapTileScale){
	
	tileSize = mapTileSize;
	tileScale = mapTileScale;
	tileWorldSize = tileSize * tileScale;

	for (int i = 0; i < tileIdCount; ++i) {
		speedMultipliers[i] = 1.0;
		frictions[i] = 1.0;
		accelerations[i] = 0.0;
	}

	std::ifstream mapFile(filePath);

	if (!mapFile.is_open()){
		Logger::Err("Failed to open map file: " + filePath);
		return;
	}


	std::vector<std::vector<int>> parsed;		//one inner vector per map row (tileEditorChange)
	std::string line;
	int lineNumber = 0;

	while (std::getline(mapFile, line)) {		 // reads one line, returns false at the end of the file
		lineNumber++;
		if (line.ends_with("\r")) {
			line.pop_back();					//  \r appares when file is made on windows
		}

		if (line.empty()) {
			continue;							//trailing newline at the end of the file
		}

		std::vector<int> values;
		std::stringstream lineStream(line);
		std::string token;
		
		try {
			while (std::getline(lineStream, token, ',')) {		// third argument = stop at ',' instead of '\n'
				const int tileId = std::stoi(token);

				if (tileId < 0 || tileId >= tileIdCount) {
					Logger::Err(filePath + " line " + std::to_string(lineNumber) + ": tile id " + std::to_string(tileId)
						+ " is out of range 0-" + std::to_string(tileIdCount - 1));
					return;
				}
				values.push_back(tileId);
			}
		}

		catch (const std::exception&){
			Logger::Err(filePath + " line " + std::to_string(lineNumber) + ": '" + token + "' is not a tile id");
			return;
		}

		if (!parsed.empty() && values.size() != parsed[0].size()) {
			Logger::Err(filePath + " line" + std::to_string(lineNumber) + ": " + std::to_string(values.size()) + " tiles, expected: " + std::to_string(parsed[0].size()));
			return;
		}

		parsed.push_back(values);
	}

	if (parsed.empty()) {
		Logger::Err("Map file is empty: " + filePath);
		return;
	}

	rows = parsed.size();
	cols = parsed[0].size();
	tileIds.assign(rows * cols, 0);
	fileName = std::filesystem::path(filePath).filename().string();

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			tileIds[Index(x, y)] = parsed[y][x];
		}
	}


}

bool TileMap::Save(const std::string& filePath) const {
	std::ofstream mapFile(filePath);

	if (!mapFile.is_open()) {
		Logger::Err("Failed to open map file: " + filePath);
		return false;
	}

	const int rowsNum = Rows();
	const int colsNum = Cols();
	for (int row = 0; row < rowsNum; ++row) {
		for (int col = 0; col < colsNum; ++col) {
			
			if (col > 0) {
				mapFile << ',';
			}
			mapFile << tileAt(col, row);

		}
		if (row < rowsNum - 1) {
			mapFile << '\n';
		}
	}
	return true;
}
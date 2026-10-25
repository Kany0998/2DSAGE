#ifndef PATHBENCHMARK_H
#define PATHBENCHMARK_H

#include "../TileMap/TileMap.h"
#include <sol/sol.hpp>
#include <vector>
#include <string>

struct Route {
	std::string name;
	int fromCol = -1;
	int fromRow = -1;
	int toCol = -1;
	int toRow = -1;
	int boundTiles = 0;		//0 = no limits
};

struct PathBenchmarkSettings {
	int runs = 1;
	std::vector<Route> routes;
	static PathBenchmarkSettings Load(sol::state& lua, const TileMap& tileMap);
};

struct SearchMode {
	std::string name;
	double gWeight;
	double hWeight;
};

void RunPathBenchmark(const PathBenchmarkSettings& pathBench, const TileMap& tileMap);

#endif // !PATHBENCHMARK_H

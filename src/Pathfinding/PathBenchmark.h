#ifndef PATHBENCHMARK_H
#define PATHBENCHMARK_H

#include "../TileMap/TileMap.h"
#include <sol/sol.hpp>
#include <vector>
#include <string>

struct Route {
	std::string name;
	int fromCol;
	int fromRow;
	int toCol;
	int toRow;
};

struct PathBenchmarkSettings {
	int runs = 1;
	std::vector<Route> routes;
	static PathBenchmarkSettings Load(sol::state& lua, const TileMap& tileMap);
};

#endif // !PATHBENCHMARK_H

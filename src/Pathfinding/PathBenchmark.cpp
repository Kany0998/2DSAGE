#include "PathBenchmark.h"
#include "Pathfinder.h"
#include "../TileMap/MovementType.h"
#include "../Logger/Logger.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iomanip>


PathBenchmarkSettings PathBenchmarkSettings::Load(sol::state& lua, const TileMap& tileMap) {
	PathBenchmarkSettings routeSettings;

	sol::optional<sol::table> table = lua["Level"]["pathfinding_benchmark"];

	if (!table) {
		return routeSettings;
	}

	const int runNumber = (*table)["runs"].get_or(1);

	if (runNumber < 1) {
		Logger::Warn("PathBenchmark: the runs < 1 :" + std::to_string(runNumber) + " which cannot be allowed, setting it to default: " + std::to_string(routeSettings.runs));
	}
	else {
		routeSettings.runs = runNumber;
	}

	sol::optional<sol::table> routesTable = (*table)["routes"];

	if (routesTable) {
		for (int i = 1; i <= static_cast<int>(routesTable->size()); ++i) {
			sol::object value = (*routesTable)[i];

			if (!value.is<sol::table>()) {
				Logger::Warn("PathBenchmark: a routes entry is not a table, skipped");
				continue;
			}
			sol::table entry = value.as<sol::table>();

			Route route;
			route.name = entry["name"].get_or(std::string(""));
			const int fromCol = entry["from"]["col"].get_or(-1);
			const int fromRow = entry["from"]["row"].get_or(-1);
			const int toCol = entry["to"]["col"].get_or(-1);
			const int toRow = entry["to"]["row"].get_or(-1);
			const int boundedTiles = entry["bound_radius_tiles"].get_or (0);

			if (!tileMap.isInside(fromCol, fromRow) || !tileMap.isInside(toCol, toRow)) {
				Logger::Warn("PathBenchmark: route: " + route.name + " is outside the map, skipped");
				continue;
			}

			if (tileMap.isBlocked(fromCol, fromRow, MovementType_Ground) || tileMap.isBlocked(toCol, toRow, MovementType_Ground)) {
				Logger::Warn("PathBenchmark: route: " + route.name + " start or end is blocked ,skipped");
				continue;
			}

			if (boundedTiles < 0) {
				Logger::Warn("PathBenchmark: route: " + route.name + " has bound_radius_tiles < 0 : "+std::to_string(boundedTiles) +" which is not allowed , chaning it to default" + std::to_string(route.boundRadiusTiles));
			}
			else {
				route.boundRadiusTiles = boundedTiles;
			}

			route.fromCol = fromCol;
			route.fromRow = fromRow;
			route.toCol = toCol;
			route.toRow = toRow;
			routeSettings.routes.push_back(route);
		}
	}
	return routeSettings;
}

void RunPathBenchmark(const PathBenchmarkSettings& pathBench, const TileMap& tileMap) {
	Pathfinder pathfinder;                 // its own, never the enemies' or the debug one
	pathfinder.smoothingEnable = false;    // measure the search only

	const double admissibleH = 1.0 / std::max(1.0, tileMap.MaxSpeedMultiplier());
	std::vector<int> path;

	const std::vector<SearchMode> modes = {
		{"Dijkstra", 1.0, 0.0},
		{"A*", 1.0, 1.0},
		{"A* admissible", 1.0, admissibleH},
		{"Greedy", 0.0, 1.0},
	};

	std::filesystem::create_directories("./benchmarks");
	const std::string fileName = "./benchmarks/pathfinding_" + std::to_string(tileMap.Cols()) + "x" + std::to_string(tileMap.Rows()) + ".csv";

	std::ofstream csv(fileName);
	if (!csv) {
		Logger::Err("PathBenchmark: could not open " + fileName + ", results go to the log only");
	}
	csv << std::fixed << std::setprecision(6);
	csv << "route,mode,found,cost,expanded,pushed,avg_ms,max_ms,reset_ms\n";

	for (auto& route : pathBench.routes) {

		auto bounds = SearchBounds{};
		if (route.boundRadiusTiles > 0) {
			bounds.minCol = std::max(0, route.fromCol - route.boundRadiusTiles);
			bounds.maxCol = std::min(tileMap.Cols()- 1, route.fromCol + route.boundRadiusTiles);
			bounds.minRow = std::max(0, route.fromRow - route.boundRadiusTiles);
			bounds.maxRow = std::min(tileMap.Rows() - 1, route.fromRow + route.boundRadiusTiles);
		}

		for (auto& mode : modes) {
			pathfinder.gWeight = mode.gWeight;
			pathfinder.hWeight = mode.hWeight;

			pathfinder.FindPath(tileMap, route.fromCol, route.fromRow, route.toCol, route.toRow, MovementType_Ground, path, bounds);

			double sumSearch = 0;
			double maxSearch = 0;
			double sumReset = 0;
			
			for (int run = 0; run < pathBench.runs; ++run) {
				pathfinder.FindPath(tileMap, route.fromCol, route.fromRow, route.toCol, route.toRow, MovementType_Ground, path, bounds);
				const auto& stats = pathfinder.LastStats();
				sumSearch += stats.searchMs;
				maxSearch = std::max(maxSearch, stats.searchMs);
				sumReset += stats.resetMs;
			}

			const auto& last = pathfinder.LastStats();
			double averageSearch = sumSearch / pathBench.runs;
			double averageReset = sumReset / pathBench.runs;
			Logger::Log(route.name + " | " + mode.name
				+ " | found " + std::to_string(last.found)
				+ " | cost " + std::to_string(last.pathCost)
				+ " | expanded " + std::to_string(last.nodesExpanded)
				+ " | pushed " + std::to_string(last.nodesPushed)
				+ " | avg " + std::to_string(averageSearch) + " ms"
				+ " | max " + std::to_string(maxSearch) + " ms"
				+ " | reset " + std::to_string(averageReset) + " ms" );

			csv << route.name << "," << mode.name << ","
				<< last.found << "," << last.pathCost << "," << last.nodesExpanded << "," << last.nodesPushed << ","
				<< averageSearch << "," << maxSearch << "," << averageReset << "\n";
		}
		Logger::Log("-----------------------------------------------------");
	}
}
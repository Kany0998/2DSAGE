#include "PathBenchmark.h"
#include "../TileMap/MovementType.h"
#include "../Logger/Logger.h"


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

			if (!tileMap.isInside(fromCol, fromRow) || !tileMap.isInside(toCol, toRow)) {
				Logger::Warn("PathBenchmark: route: " + route.name + " is outside the map, skipped");
				continue;
			}

			if (tileMap.isBlocked(fromCol, fromRow, MovementType_Ground) || tileMap.isBlocked(toCol, toRow, MovementType_Ground)) {
				Logger::Warn("PathBenchmark: route: " + route.name + " start or end is blocked ,skipped");
				continue;
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
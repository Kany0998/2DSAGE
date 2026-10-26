#include "SimulationSettings.h"
#include "../Logger/Logger.h"
#include <algorithm>

SimulationSettings SimulationSettings::Load(sol::state& lua) {
	SimulationSettings simulationSettings;
	sol::optional<sol::table> table = lua["SimulationSettings"];

	if (!table) {
		return simulationSettings;
	}

	simulationSettings.enable = (*table)["enable"].get_or(true);
	double activeTileRad = (*table)["activate_radius_tiles"].get_or(simulationSettings.activateTilesRadius);
	double deacticeTileRad = (*table)["deactivate_radius_tiles"].get_or(simulationSettings.deactivateTilesRadius);
	const double checkSec = (*table)["check_interval_seconds"].get_or(simulationSettings.checksIntervalSeconds);

	if (activeTileRad == deacticeTileRad) {
		deacticeTileRad += 6;
	}

	if (deacticeTileRad < activeTileRad) {
		Logger::Warn("SimulationSettings: activate_radius_tiles > deactivate_radius_tiles " + std::to_string(activeTileRad) + " > " + std::to_string(deacticeTileRad) + ", swapped");
		std::swap(activeTileRad, deacticeTileRad);
	}

	if (activeTileRad <= 0) {
		Logger::Warn("SimulationSettings: activate_radius_tiles <= 0: " + std::to_string(activeTileRad) + " which is not allowed changing it to default: " + std::to_string(simulationSettings.activateTilesRadius));
	}
	else {
		simulationSettings.activateTilesRadius = activeTileRad;
	}

	if (deacticeTileRad <= 0) {
		Logger::Warn("SimulationSettings: deactivate_radius_tiles <= 0: " + std::to_string(deacticeTileRad) + " which is not allowed changing it to default: " + std::to_string(simulationSettings.deactivateTilesRadius));
	}
	else {
		simulationSettings.deactivateTilesRadius = deacticeTileRad;
	}

	if (checkSec < 0.1 || checkSec > 1.0) {
		Logger::Warn("SimulationSettings: check_interval_seconds is < 0.1 or >1.0 : " + std::to_string(checkSec) + " which is not allowed value is clamped (0.25 , 1.0)");
		simulationSettings.checksIntervalSeconds = std::clamp(checkSec, 0.1, 1.0);
	}
	else {
		simulationSettings.checksIntervalSeconds = checkSec;
	}

	return simulationSettings;
	

}
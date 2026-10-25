#ifndef SIMULATIONSETTINGS_H
#define SIMULATIONSETTINGS_H

#include <sol/sol.hpp>

struct SimulationSettings {
	bool enable = true;
	double activateTilesRadius = 20.0;
	double deactivateTilesRadius = 26.0;
	double checksIntervalSeconds = 0.25;

	static SimulationSettings Load(sol::state& lua);
};

#endif // !SIMULATIONSETTINGS_H

#ifndef FLANKINGSETTINGS_H
#define FLANKINGSETTINGS_H

#include <sol/sol.hpp>

struct FlankingSettings {
	bool enable = false;
	double penaltyTilesCost = 4.0;
	int ignoreRadiusTiles = 5;

	static FlankingSettings Load(sol::state& lua);
};

#endif // !FLANKINGSETTINGS_H

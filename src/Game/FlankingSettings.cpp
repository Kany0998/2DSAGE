#include "FlankingSettings.h"
#include "../Logger/Logger.h"
#include <algorithm>

FlankingSettings FlankingSettings::Load(sol::state& lua) {
	FlankingSettings flankingSettings;
	sol::optional<sol::table> table = lua["FlankingSettings"];

	if (!table) {
		return flankingSettings;
	}

	flankingSettings.enable = (*table)["enable"].get_or(false);
	double penaltyCost = (*table)["penalty_tiles_cost"].get_or(flankingSettings.penaltyTilesCost);
	int ignoreRadius = (*table)["ignore_radius_tiles"].get_or(flankingSettings.ignoreRadiusTiles);
	double swapRankRange = (*table)["swap_rank_margin_tiles"].get_or(flankingSettings.swapRankMarginTiles);

	if (penaltyCost <= 0.0) {
		Logger::Warn("FlankingSettings: the penalty_tiles_cost <= 0.0 :" + std::to_string(penaltyCost) +
			" which is not allowed, because a negative penalty makes the octile heuristic overestimate ,changing it to default: " + std::to_string(flankingSettings.penaltyTilesCost));
	}
	else {
		flankingSettings.penaltyTilesCost = penaltyCost;
	}

	if (ignoreRadius <= 0) {
		Logger::Warn("FlankingSettings: ignore_radius_tiles <= 0 : " + std::to_string(ignoreRadius) + ", which is not allowed changing it to default: " + std::to_string(flankingSettings.ignoreRadiusTiles));
	}
	else if (ignoreRadius < 1 || ignoreRadius > 10) {
		const int clamped = std::clamp(ignoreRadius, 1, 10);
		Logger::Warn("FlankingSettings: ignore_radius_tiles = "+std::to_string(ignoreRadius) + " is outside 1 - 10, clamped to "+std::to_string(clamped)+ 
			" (below 1 the final approach to the player is penalised; above 10 flanking rarely has any effect)");
		flankingSettings.ignoreRadiusTiles = clamped;
	}
	else {
		flankingSettings.ignoreRadiusTiles = ignoreRadius;
	}
	if (swapRankRange < 1.0) {
		Logger::Warn("FlankingSettings: the swap_rank_margin_tiles < 1.0 :" + std::to_string(swapRankRange) + " which is not allowed changing it to default: " + std::to_string(flankingSettings.swapRankMarginTiles));
	}
	else {
		flankingSettings.swapRankMarginTiles = swapRankRange;
	}
	return flankingSettings;
}
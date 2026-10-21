#include "LevelAmbience.h"
#include "../Logger/Logger.h"

LevelAmbience LevelAmbience::Load(sol::state& lua) {

	LevelAmbience levelAmbience;
	
	sol::optional<sol::table> table = lua["Level"]["ambience"];
	if (!table) {
		return levelAmbience;
	}

	levelAmbience.loop = (*table)["loop"].get_or(std::string(""));

	sol::optional<sol::table> randomTable = (*table)["random"];

	if (randomTable) {
		for (const auto& [key, value] : *randomTable) {
			
			if (!value.is<sol::table>()) {
				Logger::Warn("LevelAmbience: a random entry is not a table, skipped");
				continue;
			}
			sol::table entry = value.as<sol::table>();

			AmbientSound sound;
			sound.soundId = entry["sound"].get_or(std::string(""));
			int minSeconds = entry["min_seconds"].get_or(20);
			int maxSeconds = entry["max_seconds"].get_or(30);

			if (sound.soundId.empty()) {
				Logger::Warn("LevelAmbience: no soundId, nothing to play, skipped");
				continue;
			}

			if (minSeconds < 1) {
				Logger::Warn("LevelAmbience: min seconds < 1: " + std::to_string(minSeconds) + " change it to 1");
				minSeconds = 1;
			}

			if (maxSeconds < 2) {
				Logger::Warn("LevelAmbience: min seconds < 2: " + std::to_string(maxSeconds) + " change it to 2");
				maxSeconds = 2;
			}

			if (minSeconds > maxSeconds) {
				Logger::Warn("LevelAmbience: min > max : " + std::to_string(minSeconds) + " > " + std::to_string(maxSeconds) + " ,swapped");
				std::swap(minSeconds, maxSeconds);
			}

			sound.minMs = static_cast<Uint32>(minSeconds * 1000);
			sound.maxMs = static_cast<Uint32>(maxSeconds * 1000);
			
			levelAmbience.random.push_back(sound);
		}
	}

	return levelAmbience;
}
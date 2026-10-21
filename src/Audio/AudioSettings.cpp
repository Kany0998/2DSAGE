#include "AudioSettings.h"
#include "../Logger/Logger.h"
#include <algorithm>

static float ReadVolume(const sol::table& table, const char* name, float fallback) {
	float value = table[name].get_or(fallback);

	if (value < 0.0f || value > 1.0f) {
		Logger::Warn("AudioSettings : " + std::string(name) + " must be => 0.0 or <=1.0, calmping the value");
		value = std::clamp(value, 0.0f, 1.0f);
	}
	return value;
}

AudioSettings AudioSettings::Load(sol::state& lua) {
	AudioSettings audioSettings;

	sol::optional<sol::table> table = lua["AudioSettings"];
	if (!table) {
		return audioSettings;
	}

	audioSettings.master = ReadVolume(*table, "master", audioSettings.master);
	audioSettings.music = ReadVolume(*table, "music", audioSettings.music);
	audioSettings.effects = ReadVolume(*table, "effects", audioSettings.effects);
	audioSettings.ambience = ReadVolume(*table, "ambience", audioSettings.ambience);
	audioSettings.defaultStepSound = (*table)["default_step_sound"].get_or(std::string(""));

	float hearingRangeTile = (*table)["hearing_range_tiles"].get_or(audioSettings.hearingRangeTiles);
	if (hearingRangeTile <= 0.0) {
		Logger::Warn("AudioSettings: hearing_range_tiles <= 0.0 : "+std::to_string(hearingRangeTile) +" , changing it to default "+ std::to_string(audioSettings.hearingRangeTiles));
	}

	else {
		audioSettings.hearingRangeTiles = hearingRangeTile;
	}
	int numChannels = (*table)["channels"].get_or(audioSettings.channels);

	if (numChannels < 1) {
		Logger::Warn("AudioSettings: number of channels is < 1: " + std::to_string(numChannels) + ", changing it to default " + std::to_string(audioSettings.channels));
	}

	else {
		if (numChannels > 128) {
			Logger::Warn("AudioSettings: number of channels is: " + std::to_string(numChannels) + " which is > 128, might be a typo");
		}
		audioSettings.channels = numChannels;
	}

	int repGapMs = (*table)["repeat_gap_ms"].get_or(audioSettings.repeatGapMs);

	if (repGapMs < 0) {
		Logger::Warn("AudioSettings: repeat_gap_ms < 0: " + std::to_string(repGapMs) + " which is not allowed, chaning it to: " + std::to_string(audioSettings.repeatGapMs));
	}

	else {
		audioSettings.repeatGapMs = repGapMs;
	}

	int loopSources = (*table)["max_loop_sources"].get_or(audioSettings.maxLoopSources);
	
	if (loopSources < 0) {
		Logger::Warn("AudioSettings: max_loop_sources < 0: " + std::to_string(loopSources) + " which is not allowed, chaning it to engine default: " + std::to_string(audioSettings.maxLoopSources));
	}
	else {
		if (loopSources > 128) {
			Logger::Warn("AudioSettings: max_loop_sources > 128: " + std::to_string(loopSources) + " make sure this is not a typo");
		}
		audioSettings.maxLoopSources = loopSources;
	}

	sol::optional<sol::table> uiTable = (*table)["ui"];
	if (uiTable) {
		for (const auto& [key, value] : *uiTable) {

			if (key.get_type() != sol::type::string) {
				Logger::Warn("AudioSettings: a ui has no id");
				continue;
			}
			std::string id = key.as<std::string>();

			if (!value.is<std::string>()) {
				Logger::Warn("AudioSettings: " + id + ": not a string, skipped");
				continue;
			}

			audioSettings.ui[id] = value.as<std::string>();
		}
	}

	return audioSettings;
}
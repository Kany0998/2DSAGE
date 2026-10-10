#include "InventoryUISettings.h"
#include "../Logger/Logger.h"
#include <string>

//One number from the Lua InventoryUI table, or the fallback if it is missing or not above 0
static int ReadPositive(const sol::table& table, const char* name, int fallback) {

	int value = table[name].get_or(fallback);

	if (value <= 0) {
		Logger::Warn("inventory ui: " + std::string(name) + " must be > 0, keeping " + std::to_string(fallback));
		return fallback;
	}

	return value;
}

InventoryUISettings InventoryUISettings::Load(sol::state& lua) {

	InventoryUISettings settings;

	sol::optional<sol::table> table = lua["InventoryUI"];
	if (!table) {
		return settings;
	}

	settings.iconSize = ReadPositive(*table, "icon_size", settings.iconSize);
	settings.iconScale = ReadPositive(*table, "icon_scale", settings.iconScale);
	settings.backpackColumns = ReadPositive(*table, "backpack_columns", settings.backpackColumns);

	return settings;
}
#include "AbilityDatabase.h"
#include "../Logger/Logger.h"

void AbilityDatabase::Load(sol::state& lua, const std::string& path, const ItemDatabase& itemDatabase) {
	sol::protected_function_result result = lua.safe_script_file(path, sol::script_pass_on_error);

	if (!result.valid()) {
		sol::error err = result;
		Logger::Err("Abilities: could not run " + path + ": " + err.what());
		return;
	}

	sol::optional<sol::table> abilityTable = lua["Abilities"];

	if (!abilityTable) {
		Logger::Warn("Abilities: " + path + " defines no Abilities table");
		return;
	}

	for (const auto& [key, value] : *abilityTable) {

		if (key.get_type() != sol::type::string) {
			Logger::Warn("Abilities: an ability has no id (write it as some_id = { ... }), skipped");
			continue;
		}
		std::string id = key.as<std::string>();

		if (!value.is<sol::table>()) {
			Logger::Warn("Abilities: " + id + ": not a table, skipped");
			continue;
		}
		sol::table ability = value.as<sol::table>();

		sol::object onCast = ability["on_cast"];
		if (onCast.get_type() != sol::type::function) {
			Logger::Warn("Abilities: " + id + ": on_cast must be a function, skipped");
			continue;
		}

		AbilityDefinition definition;
		definition.id = id;
		definition.onCast = onCast.as<sol::protected_function>();
		definition.data = ability;

		abilities[id] = std::move(definition);
	}

	for (const auto& [itemId, item] : itemDatabase.All()) {
		if (item.ability.empty()) {
			continue;
		}

		const AbilityDefinition* ability = Find(item.ability);

		if (ability == nullptr) {
			Logger::Warn("Abilities: item " + itemId + " uses unknown ability " + item.ability + ", skipped");
			continue;
		}

		//the item's own ability_stats, or an empty table if it overrides nothing
		sol::optional<sol::table> overrides = lua["Items"][itemId]["ability_stats"];
		sol::table stats = overrides ? *overrides : lua.create_table();

		sol::table meta = lua.create_table();
		meta["__index"] = ability->data;

		//keys missing from stats are looked up in the ability's table when read
		stats[sol::metatable_key] = meta;

		const double manaCost = stats["mana_cost"].get_or(0.0);
		if (manaCost < 0) {
			Logger::Warn("Abilities: item " + itemId + ": mana_cost " + std::to_string(manaCost) + " is negative, using 0");
			stats["mana_cost"] = 0;
		}

		const double seconds = stats["cooldown_seconds"].get_or(0.0);

		if (seconds < 0) {
			Logger::Warn("Abilities: item " + itemId + ": cooldown_seconds " + std::to_string(seconds) + " is negative, using 0");
			stats["cooldown_seconds"] = 0;
		}

		itemStats[itemId] = stats;
	}
}

const AbilityDefinition* AbilityDatabase::Find(const std::string& id) const {
	auto it = abilities.find(id);
	if (it == abilities.end()) {
		return nullptr;
	}
	return &it->second;
}

const sol::table* AbilityDatabase::StatsForItem(const std::string& itemId) const {
	auto it = itemStats.find(itemId);
	if (it == itemStats.end()) {
		return nullptr;
	}
	return &it->second;
}
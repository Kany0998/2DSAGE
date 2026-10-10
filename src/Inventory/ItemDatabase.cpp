#include "ItemDatabase.h"
#include "../Logger/Logger.h"
#include <utility>

void ItemDatabase::Load(sol::state& lua, const std::string& path) {
	sol::protected_function_result result = lua.safe_script_file(path, sol::script_pass_on_error);

	if (!result.valid()) {
		sol::error err = result;
		Logger::Err("items: could not run " + path + ": " + err.what());
		return;
	}

	sol::optional<sol::table> itemsTable = lua["Items"];

	if (!itemsTable) {
		Logger::Warn("items: " + path + " defines no Items table");
		return;
	}

	for (const auto& [key, value] : *itemsTable) {

		if (key.get_type() != sol::type::string) {
			Logger::Warn("items: an item has no id (write it as some_id = { ... }), skipped");
			continue;
		}
		std::string id = key.as<std::string>();

		if (!value.is<sol::table>()) {
			Logger::Warn("items: " + id + ": not a table, skipped");
			continue;
		}
		sol::table item = value.as<sol::table>();

		ItemDefinition definition;
		definition.id = id;
		definition.name = item["name"].get_or(id);
		definition.icon = item["icon"].get_or(std::string(""));
		definition.tag = item["tag"].get_or(std::string(""));
		if (definition.tag.empty()) {
			Logger::Warn("items: " + id + ": has no tag");
		}

		definition.ability = item["ability"].get_or(std::string(""));
		if (definition.tag == "special_ability" && definition.ability.empty()) {
			Logger::Warn("items: " + id + ": has no ability");
		}

		//bonuses: { defense = 60, speed = -10 } -> bonus[Skill_Defense] = 60, bonus[Skill_Speed] = -10
		sol::optional<sol::table> bonuses = item["bonuses"];
		if (bonuses) {
			for (const auto& [statKey, statValue] : *bonuses) {
				std::string statName = statKey.as<std::string>();

				int statIndex = -1;
				for (int i = 0; i < Skill_Count; i++) {
					if (statName == SkillLuaNames[i]) {
						statIndex = i;
						break;
					}
				}

				if (statIndex == -1) {
					Logger::Warn("items: " + id + ": unknown bonus '" + statName + "', skipped");
					continue;
				}
				if (statValue.get_type() != sol::type::number) {
					Logger::Warn("items: " + id + ": bonus '" + statName + "' is not a number, skipped");
					continue;
				}

				definition.bonus[statIndex] = statValue.as<int>();
			}
		}

		//weapon: only read for items tagged "weapon"; any field missing keeps the WeaponStats default
		sol::optional<sol::table> weapon = item["weapon"];
		if (definition.tag == "weapon") {
			if (!weapon) {
				Logger::Warn("items: " + id + ": tagged 'weapon' but has no weapon table");
			}

			else {
				definition.isWeapon = true;
				WeaponStats& w = definition.weapon;
				const WeaponStats defaults;

				w.minDamage = (*weapon)["min_damage"].get_or(w.minDamage);
				w.maxDamage = (*weapon)["max_damage"].get_or(w.maxDamage);
				w.projectileTexture = (*weapon)["projectile_texture"].get_or(w.projectileTexture);
				w.projectileSpeed = (*weapon)["projectile_speed"].get_or(w.projectileSpeed);
				w.shotPerSecond = (*weapon)["shots_per_second"].get_or(w.shotPerSecond);
				w.rangeTiles = (*weapon)["range_tiles"].get_or(w.rangeTiles);

				if (w.minDamage > w.maxDamage) {
					Logger::Warn("items: " + id + ": min_damage " + std::to_string(w.minDamage) +
						" > max_damage " + std::to_string(w.maxDamage) + ", swapped");
					std::swap(w.minDamage, w.maxDamage);
				}

				if (w.projectileSpeed <= 0) {
					Logger::Warn("items: " + id + ": projectile_speed must be > 0, using default");
					w.projectileSpeed = defaults.projectileSpeed;
				}

				if (w.shotPerSecond <= 0) {
					Logger::Warn("items: " + id + ": shots_per_second must be > 0, using default");
					w.shotPerSecond = defaults.shotPerSecond;
				}

				if (w.rangeTiles <= 0) {
					Logger::Warn("items: " + id + ": range_tiles must be > 0, using default");
					w.rangeTiles = defaults.rangeTiles;
				}
			}
		}
		else if (weapon) {
			Logger::Warn("items: " + id + ": has a weapon table but is not tagged 'weapon', ignored");
		}

		items[id] = definition;
	}

	Logger::Log("items: loaded " + std::to_string(items.size()) + " items from " + path);
}

const ItemDefinition* ItemDatabase::Find(const std::string& id) const {
	auto it = items.find(id);
	if (it == items.end()) {
		return nullptr;
	}
	return &it->second;
}
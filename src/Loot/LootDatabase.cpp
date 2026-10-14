#include "LootDatabase.h"
#include "../Logger/Logger.h"
#include <algorithm>


void LootDatabase::Load(sol::state& lua, const std::string& path, const ItemDatabase& items) {
	sol::protected_function_result result = lua.safe_script_file(path, sol::script_pass_on_error);

	if (!result.valid()) {
		sol::error err = result;
		Logger::Err("loot: could not run " + path + ": " + err.what());
		return;
	}

	sol::optional<sol::table> lootBags = lua["LootBags"];

	if (!lootBags) {
		Logger::Warn("loot: " + path + " defines no LootBags table");
		return;
	}

	for (const auto& [key, value] : *lootBags) {

		if (key.get_type() != sol::type::string) {
			Logger::Warn("loot: a loot bag has no id (write it as some_id(string) = { ... }), skipped");
			continue;
		}
		std::string id = key.as<std::string>();

		if (!value.is<sol::table>()) {
			Logger::Warn("loot: " + id + ": not a table, skipped");
			continue;
		}
		sol::table bag = value.as<sol::table>();

		BagTier bagTier;
		bagTier.id = id;
		bagTier.texture = bag["texture"].get_or(std::string(""));
		bagTier.rank = bag["rank"].get_or(bagTier.rank);
		bagTier.lifetimeMs = static_cast<int>(bag["lifetime_seconds"].get_or(30.0f) * 1000.0f);
		bagTier.dropSound = bag["drop_sound"].get_or(std::string(""));
		bagTier.announceImage = bag["announce_image"].get_or(std::string(""));
		bagTier.announceMs = static_cast<int>(bag["announce_seconds"].get_or(4.0f) * 1000.0f);
		bagTier.announceScale = bag["announce_scale"].get_or(bagTier.announceScale);
		bagTier.announceSound = bag["announce_sound"].get_or(std::string(""));

		if (bagTier.texture.empty()) {
			Logger::Warn("loot: " + id + " has not texture, skipped");
			continue;
		}

		if (bagTier.rank <= 0) {
			Logger::Warn("loot: " + id + " has rank: " + std::to_string(bagTier.rank) + " <= 0 ,new value assign rank = " + std::to_string(BagTier{}.rank));
			bagTier.rank = BagTier{}.rank;
		}

		if (bagTier.lifetimeMs <= 0) {
			Logger::Warn("loot: " + id + " has lifetime_seconds: " + std::to_string(bagTier.lifetimeMs / 1000) + " <= 0 ,new value assign lifetime_seconds = " + std::to_string(BagTier{}.lifetimeMs / 1000) + " seconds");
			bagTier.lifetimeMs = BagTier{}.lifetimeMs;
		}

		if (bagTier.announceMs <= 0) {
			Logger::Warn("loot: " + id + " has announce_seconds: " + std::to_string(bagTier.announceMs / 1000) + " <= 0 ,new value assign announce_seconds = " + std::to_string(BagTier{}.announceMs / 1000) + " seconds");
			bagTier.announceMs = BagTier{}.announceMs;
		}

		if (bagTier.announceScale <= 0) {
			Logger::Warn("loot: " + id + " has announce_scale: " + std::to_string(bagTier.announceScale) + " <= 0 ,new value assign announce_scale = " + std::to_string(BagTier{}.announceScale));
			bagTier.announceScale = BagTier{}.announceScale;
		}

		bags[id] = bagTier;
	}

	sol::optional<sol::table> lootTables = lua["LootTables"];
	if (lootTables) {
		for (const auto& [tableKey, tableValue] : *lootTables) {
			if (tableKey.get_type() != sol::type::string) {
				Logger::Warn("loot: a loot table has no id (write it as some_id(string) = { ... }), skipped");
				continue;
			}
			std::string tableName = tableKey.as<std::string>();

			if (!tableValue.is<sol::table>()) {
				Logger::Warn("loot: table '" + tableName + "' is not a list of lines, skipped");
				continue;
			}

			sol::table lines = tableValue.as<sol::table>();
			for (const auto& [lineKey, lineValue] : lines) {

				if (lineValue.get_type() != sol::type::table) {
					Logger::Warn("loot: table '" + tableName + "' line " + std::to_string(lineKey.as<int>()) +" is not { item = ..., chance = ... }, skipped");
					continue;
				}

				sol::table line = lineValue.as<sol::table>();

				LootEntry lootEntry;
				lootEntry.itemId = line["item"].get_or(std::string(""));
				lootEntry.chance = line["chance"].get_or(0.0f);

				if (items.Find(lootEntry.itemId) == nullptr) {
					Logger::Warn("loot: table '" + tableName + "' has unknown item '" + lootEntry.itemId + "', line skipped");
					continue;
				}

				if (lootEntry.chance < 0.0f || lootEntry.chance > 1.0f) {
					Logger::Warn("loot: table '" + tableName + "' item '" + lootEntry.itemId + "' has chance " +
						std::to_string(lootEntry.chance) + ", must be 0-1, clamped");
					lootEntry.chance = std::clamp(lootEntry.chance, 0.0f, 1.0f);
				}

				tables[tableName].push_back(lootEntry);
			}
		}
	}

	//settings: missing table or field keeps the default from the header
	sol::optional<sol::table> lootSettings = lua["LootSettings"];
	if (lootSettings) {
		sol::table settings = *lootSettings;

		const float defaultRange = pickupRangeTiles;
		const int defaultBlink = blinkMs;
		const int defaultScale = bagScale;

		pickupRangeTiles = settings["pickup_range_tiles"].get_or(pickupRangeTiles);
		blinkMs = static_cast<int>(settings["blink_seconds"].get_or(blinkMs / 1000.0f) * 1000.0f);
		bagScale = settings["bag_scale"].get_or(bagScale);
		defaultBag = settings["default_bag"].get_or(std::string(""));
		playerDropBag = settings["player_drop_bag"].get_or(std::string(""));

		if (pickupRangeTiles <= 0.0f) {
			Logger::Warn("loot: pickup_range_tiles must be > 0, using " + std::to_string(defaultRange));
			pickupRangeTiles = defaultRange;
		}

		if (blinkMs < 0) {
			Logger::Warn("loot: blink_seconds must not be negative, using " + std::to_string(defaultBlink / 1000.0f));
			blinkMs = defaultBlink;
		}

		if (bagScale <= 0) {
			Logger::Warn("loot: bag_scale must be > 0, using " + std::to_string(defaultScale));
			bagScale = defaultScale;
		}

		
		
	}
	//default bag: must name a real tier; otherwise the lowest-ranked tier takes its place
	if (FindBag(defaultBag) == nullptr && !bags.empty()) {
		std::string lowestId;
		int lowestRank = 0;
		for (const auto& [id, tier] : bags) {
			if (lowestId.empty() || tier.rank < lowestRank) {
				lowestId = id;
				lowestRank = tier.rank;
			}
		}
		if (!defaultBag.empty()) {
			//set but wrong: tell the author; not set at all is allowed
			Logger::Warn("loot: default_bag '" + defaultBag + "' is not a bag, using '" + lowestId + "'");
		}
		defaultBag = lowestId;
	}

	//player drop bag: optional; a wrong name falls back to the default bag
	if (!playerDropBag.empty() && FindBag(playerDropBag) == nullptr) {
		Logger::Warn("loot: player_drop_bag '" + playerDropBag + "' is not a bag, using '" + defaultBag + "'");
		playerDropBag = defaultBag;
	}

	//items whose bag names no tier: they fall back to the default, but the author should know
	for (const auto& [itemId, item] : items.All()) {
		if (item.bag.empty()) {
			continue;
		}

		if (FindBag(item.bag) == nullptr) {
			Logger::Warn("loot: item " + item.id + " has bag "+ item.bag + ", no such bag - using " + defaultBag);
		}
	}

	Logger::Log("loot: loaded " + std::to_string(bags.size()) + " bags and " + std::to_string(tables.size()) + " tables from " + path);
}


const BagTier* LootDatabase::FindBag(const std::string& id) const {
	auto bag = bags.find(id);
	if (bag == bags.end()) {
		return nullptr;
	}
	return &bag->second;
}

const BagTier* LootDatabase::PlayerDropBag() const {
	if (playerDropBag.empty()) {
		return nullptr;
	}
	return FindBag(playerDropBag);
}

const std::vector<LootEntry>* LootDatabase::FindTable(const std::string& id) const {
	auto table = tables.find(id);
	if (table == tables.end()) {
		return nullptr;
	}
	return &table->second;
}

const BagTier* LootDatabase::BagFor(const ItemDefinition& item) const {
	const BagTier* own = FindBag(item.bag);		//the item's own tier or nullptr
	if (own != nullptr) {
		return own;
	}

	return FindBag(defaultBag);		//the default tier or nullptr if there are no tiers at all
}
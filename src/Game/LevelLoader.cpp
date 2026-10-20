#include "./LevelLoader.h"
#include "./Game.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"	
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyBoardControlledComponent.h"
#include "../Components/CameraHollderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/ManaComponent.h"	
#include "../Components/ProjectileComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/ScriptComponent.h"
#include "../Components/ProgressionComponent.h"
#include "../Components/ExperienceRewardComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Components/PushableComponent.h"
#include "../Components/AIComponent.h"
#include "../Components/BackpackComponent.h"
#include "../Components/EquipmentComponent.h"
#include "../Components/LootDropComponent.h"
#include "../Components/SoundComponent.h"
#include "../TileMap/TileMap.h"
#include "../TileMap/MovementType.h"
#include "../Inventory/ItemDatabase.h"
#include "../Inventory/InventoryRules.h"
#include <sol/sol.hpp>
#include <string>
#include <algorithm>
#include "../Logger/Logger.h"

LevelLoader::LevelLoader()
{
	Logger::Log("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader()
{
	Logger::Log("LevelLoader destructor called!");
}

void LevelLoader::LoadLevel(sol::state& lua,const std::unique_ptr<Registry>& registry,
	const std::unique_ptr<AssetStore>& assetStore,TileMap& tileMap,const ItemDatabase& itemDatabase,const LootDatabase& lootDatabase,SDL_Renderer* renderer,int levelNumber)
{
	sol::load_result check = lua.load_file("./assets/scripts/luaLoadLevel" + std::to_string(levelNumber) + ".lua");
	//check the syntax to see if it is valid
	if (!check.valid())
	{
		sol::error err = check;
		std::string errorMassage = err.what();
		Logger::Err("Error loading lua script: " + errorMassage);
		return;
	}
	//Load Entities from ./assets/scripts/luaLoadLevel1.lua
	lua.script_file("./assets/scripts/luaLoadLevel" + std::to_string(levelNumber) + ".lua");
	
	//read the data form the current level
	sol::table level = lua["Level"];

	//Read level assets
	sol::table assets = level["assets"];

	int i = 0;
	while (true)
	{
		sol::optional<sol::table> hasAsset = assets[i];

		if (hasAsset == sol::nullopt)
		{
			break;
		}
		sol::table asset = assets[i];
		std::string assetType = asset["type"];
		std::string assetId = asset["id"];

		if (assetType == "texture"){
			assetStore->AddTexture(renderer, assetId, asset["file"]);
		}

		if (assetType == "font"){
			assetStore->AddFont(assetId, asset["file"], asset["font_size"]);
		}

		if (assetType == "sound") {
			assetStore->AddSound(assetId, asset["file"]);
		}
		if (assetType == "music") {
			assetStore->AddMusic(assetId, asset["file"]);
		}
		i++;
	}

	//Load level TileMap
	sol::table map = level["tilemap"];
	std::string mapFilePath = map["map_file"];
	std::string mapTextureAssetId = map["texture_asset_id"];
	int mapNumRows = map["mapNumRows"];
	int mapNumCols = map["mapNumCols"];
	int tileSize = map["tileSize"];
	double tileScale = map["tileScale"];

	tileMap.load(mapFilePath, mapNumRows, mapNumCols, tileSize, tileScale);
	tileMap.setTextureAssetId(mapTextureAssetId);

	//Read the tile properties from the lua file
	sol::table tileProperties = map["tile_properties"];
	int tileIndex = 0;

	while (true)
	{
		sol::optional<sol::table> hasTileProperty = tileProperties[tileIndex];
		if (hasTileProperty == sol::nullopt)
		{
			break;
		}

		sol::table tileProperty = tileProperties[tileIndex];

		int tileId = tileProperty["tile_id"];
		double speedMultiplier = tileProperty["speed_multiplier"].get_or(1.0);
		double friction = tileProperty["friction"].get_or(1.0);
		double acceleration = tileProperty["acceleration"].get_or(0.0);
		bool blocksGround = tileProperty["blocks_ground"].get_or(false);
		bool blocksFlying = tileProperty["blocks_flying"].get_or(false);
		bool blocksProjectile = tileProperty["blocks_projectiles"].get_or(false);
		std::string stepSound = tileProperty["step_sound"].get_or(std::string(""));

		if (speedMultiplier == 0.0) {
			Logger::Warn("Your speed multiplier for tile: " + std::to_string(tileId) + ", is equal to :" + std::to_string(speedMultiplier) +
				" meaning it would trap everything with movement type in it: which may be a typo");
		}

		int mask = 0;
		if (blocksGround)
		{
			mask |= MovementType_Ground;
		}

		if (blocksFlying)
		{
			mask |= MovementType_Flying;
		}

		if (blocksProjectile)
		{
			mask |= MovementType_Projectile;
		}

		tileMap.setBlockMask(tileId, mask);
		tileMap.setSpeedMultiplier(tileId, speedMultiplier);
		tileMap.setAcceleration(tileId, acceleration);
		tileMap.setFriction(tileId, friction);
		tileMap.setStepSound(tileId, stepSound);
		tileIndex++;
	}
	

	Game::mapWidth = mapNumCols * tileSize * tileScale;
	Game::mapHeight = mapNumRows * tileSize * tileScale;

	//Read the level entites and Components
	sol::table entites = level["entities"];
	i = 0;

	while (true)
	{
		sol::optional<sol::table> hasEntity = entites[i];
		if (hasEntity == sol::nullopt)
		{
			break;
		}

		sol::table entity = entites[i];
		Entity newEntity = registry->CreateEntity();

		//Tags
		sol::optional<std::string> tag = entity["tag"];
		if (tag != sol::nullopt)
		{
			newEntity.Tag(entity["tag"]);
		}

		//Group
		sol::optional<std::string> group = entity["group"];
		if (group != sol::nullopt)
		{
			newEntity.Group(entity["group"]);
		}

		//Componets
		sol::optional<sol::table> hasComponents = entity["components"];
		if (hasComponents != sol::nullopt)
		{
			//transform
			sol::optional<sol::table> transform = entity["components"]["transform"];
			if (transform != sol::nullopt)
			{
				const glm::vec2 position{
					entity["components"]["transform"]["position"]["x"],
					entity["components"]["transform"]["position"]["y"]
				};

				const glm::vec2 scale{
					entity["components"]["transform"]["scale"]["x"].get_or(1.0),
					entity["components"]["transform"]["scale"]["y"].get_or(1.0)
				};

				const double rotation =
					entity["components"]["transform"]["rotation"].get_or(0.0);

				newEntity.AddComponent<TransformComponent>(
					position,
					scale,
					rotation
				);
			}

			//RigidBody
			sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
			if (rigidbody != sol::nullopt)
			{
				double mass = entity["components"]["rigidbody"]["mass"].get_or(1.0);

				if (mass <= 0.0)
				{
					Logger::Warn("mass " + std::to_string(mass) + " is not usable: a pusher would be flung away from it. Using 1.0");
					mass = 1.0;
				}

				newEntity.AddComponent<RigidBodyComponent>(
					glm::vec2(
						entity["components"]["rigidbody"]["velocity"]["x"],
						entity["components"]["rigidbody"]["velocity"]["y"]
					),
					mass
				);
			}

			//Sprite
			sol::optional<sol::table> sprite = entity["components"]["sprite"];
			if (sprite != sol::nullopt)
			{
				newEntity.AddComponent<SpriteComponent>(
					entity["components"]["sprite"]["texture_asset_id"],
					entity["components"]["sprite"]["width"],
					entity["components"]["sprite"]["height"],
					entity["components"]["sprite"]["layer"].get_or(1),
					entity["components"]["sprite"]["fixed"].get_or(false),
					entity["components"]["sprite"]["src_rect_x"].get_or(0.0),
					entity["components"]["sprite"]["src_rect_y"].get_or(0.0)
				);
			}

			//animation
			sol::optional<sol::table> animation = entity["components"]["animation"];
			if (animation != sol::nullopt)
			{
				newEntity.AddComponent<AnimationComponent>(
					entity["components"]["animation"]["num_frames"].get_or(1),
					entity["components"]["animation"]["speed_rate"].get_or(1)
				);
			}

			//BoxCollider
			sol::optional<sol::table> collider = entity["components"]["boxcollider"];
			if (collider != sol::nullopt)
			{
				newEntity.AddComponent<BoxColliderComponent>(
					entity["components"]["boxcollider"]["width"],
					entity["components"]["boxcollider"]["height"],
					glm::vec2(
						entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
						entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
					)
				);
			}

			//Health
			sol::optional<sol::table> health = entity["components"]["health"];
			if (health != sol::nullopt)
			{
				newEntity.AddComponent<HealthComponent>(
					static_cast<int>(entity["components"]["health"]["health_points"].get_or(100)),
					static_cast<int>(entity["components"]["health"]["max_health_points"].get_or(100))
				);
			}

			//Mana
			sol::optional<sol::table> mana = entity["components"]["mana"];
			if (mana != sol::nullopt)
			{
				newEntity.AddComponent<ManaComponent>(
					static_cast<int>(entity["components"]["mana"]["mana_percentage"].get_or(100)),
					static_cast<int>(entity["components"]["mana"]["max_mana_points"].get_or(100))
				);
			}
			//Attributes
			sol::optional<sol::table> attributes = entity["components"]["attributes"];
			if(attributes != sol::nullopt)
			{
				newEntity.AddComponent<AttributesComponent>(
					static_cast<int>(entity["components"]["attributes"]["attack"].get_or(10)),
					static_cast<int>(entity["components"]["attributes"]["defense"].get_or(0)),
					static_cast<int>(entity["components"]["attributes"]["wisdom"].get_or(10)),
					static_cast<int>(entity["components"]["attributes"]["vitality"].get_or(10)),
					static_cast<int>(entity["components"]["attributes"]["speed"].get_or(10)),
					static_cast<int>(entity["components"]["attributes"]["dexterity"].get_or(10))
				);
			}

			//Progression
			sol::optional<sol::table> progression = entity["components"]["progression"];
			if (progression != sol::nullopt)
			{
				newEntity.AddComponent<ProgressionComponent>(
					static_cast<int>(entity["components"]["progression"]["level"].get_or(1)),
					static_cast<int>(entity["components"]["progression"]["experience"].get_or(0)),
					//0 means "work it out from experience_base" - see ProgressionComponent
					static_cast<int>(entity["components"]["progression"]["experience_to_next_level"].get_or(0)),
					static_cast<int>(entity["components"]["progression"]["skill_points"].get_or(0)),
					static_cast<int>(entity["components"]["progression"]["max_level"].get_or(100)),
					static_cast<int>(entity["components"]["progression"]["experience_base"].get_or(100)),
					static_cast<float>(entity["components"]["progression"]["experience_growth"].get_or(1.15))
				);
			}

			//ExperienceReward
			sol::optional<sol::table> experienceReward = entity["components"]["experience_reward"];
			if (experienceReward != sol::nullopt)
			{
				newEntity.AddComponent<ExperienceRewardComponent>(
					static_cast<int>(entity["components"]["experience_reward"]["experience"].get_or(0))
				);
			}

			//ProjectileEmitter
			sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
			if (projectileEmitter != sol::nullopt)
			{

				const double tileWorldSize = tileMap.TileWorldSize();

				const double attack_range = entity["components"]["projectile_emitter"]["attack_range"].get_or(4.0) * tileWorldSize;


				newEntity.AddComponent<ProjectileEmitterComponent>(
					glm::vec2(
						entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
						entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
					),
					static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1.0) * 1000),
					static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10.0) * 1000),
					static_cast<int>(entity["components"]["projectile_emitter"]["projectile_damage"].get_or(10)),
					entity["components"]["projectile_emitter"]["aim_at_player"].get_or(false),
					attack_range,
					entity["components"]["projectile_emitter"]["friendly"].get_or(false),
					entity["components"]["projectile_emitter"]["projectile_texture"].get_or(std::string("bullet-texture")),
					entity["components"]["projectile_emitter"]["shot_sound"].get_or(std::string(""))
				);
			}

			//MovementType
			sol::optional<sol::table> movementType = entity["components"]["movement_type"];
			if (movementType != sol::nullopt)
			{
				std::string typeName = entity["components"]["movement_type"]["type"].get_or(std::string("ground"));

				int type = MovementType_Ground;
				if(typeName == "flying")
				{
					type = MovementType_Flying;
				}

				newEntity.AddComponent<MovementTypeComponent>(type);
			}

			//Pushable
			sol::optional<sol::table> pushable = entity["components"]["pushable"];
			if (pushable != sol::nullopt)
			{
				newEntity.AddComponent<PushableComponent>();
			}

			//AIComponent
			sol::optional<sol::table> AI = entity["components"]["ai"];
			if (AI != sol::nullopt)
			{
				if (newEntity.HasComponent<TransformComponent>() && newEntity.HasComponent<SpriteComponent>()) {
					const auto& transform = newEntity.GetComponent<TransformComponent>();
					const auto& sprite = newEntity.GetComponent<SpriteComponent>();

					const glm::vec2 spawnPoint = {
						transform.position.x + sprite.width * transform.scale.x / 2.0,
						transform.position.y + sprite.height * transform.scale.y / 2.0
					};


					const double tileWorldSize = tileMap.TileWorldSize();

					double detection_range = entity["components"]["ai"]["detection_range"].get_or(6.0) * tileWorldSize;
					double leash_range = entity["components"]["ai"]["leash_range"].get_or(20.0) * tileWorldSize;

					newEntity.AddComponent<AIComponent>(
						spawnPoint,
						detection_range,
						entity["components"]["ai"]["stop_distance"].get_or(1.0)* tileWorldSize,
						leash_range,
						entity["components"]["ai"]["move_speed"].get_or(100.0),
						entity["components"]["ai"]["patrol_radius"].get_or(0.0) * tileWorldSize,
						entity["components"]["ai"]["patrol_pause"].get_or(0.0)
					);

					if (leash_range < detection_range * 1.2) {
						Logger::Warn("leash_range: " + std::to_string(leash_range) + " is shorter than detection_range: " + std::to_string(detection_range) + " * 1,2 : the enemy will give up before it loses sight of the player.");
					}
					const auto& emitter = newEntity.GetComponent<ProjectileEmitterComponent>();

					if (detection_range < emitter.attackRange) {
						Logger::Warn("attack_range: " + std::to_string(emitter.attackRange) + " is greater than detection_range: " + std::to_string(detection_range) + " which may be a typo");
					}

				}
				else {
					Logger::Err("Entity needs sprite and transform components for AI");
				}
				

			}

			//CameraFollow
			sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
			if (cameraFollow != sol::nullopt)
			{
				newEntity.AddComponent<CameraHollderComponent>();
			}

			//Sound
			sol::optional<sol::table> sounds = entity["components"]["sounds"];
			if (sounds != sol::nullopt)
			{
				newEntity.AddComponent<SoundComponent>();
				auto& sound = newEntity.GetComponent<SoundComponent>();

				int min = (*sounds)["idle_min_seconds"].get_or(10);
				int max = (*sounds)["idle_max_seconds"].get_or(20);

				if (min < 1) {
					Logger::Warn("Loader: idle_min < 1: " + std::to_string(min) + " which can cause problems, new value assigned: 1 second");
					min = 1;
				}

				if (max < 2) {
					Logger::Warn("Loader: idle_max < 2: " + std::to_string(max) + " which can cause problems, new value assigned: 2 second");
					max = 2;
				}

				if (min > max) {
					Logger::Warn("Loader: idle_min > idle_max: " + std::to_string(min) + " >" + std::to_string(max) + " the values are swapped");
					std::swap(min, max);
				}

				sound.hurt = (*sounds)["hurt"].get_or(std::string(""));
				sound.death = (*sounds)["death"].get_or(std::string(""));
				sound.levelUp = (*sounds)["level_up"].get_or(std::string(""));
				sound.cast = (*sounds)["cast"].get_or(std::string(""));
				sound.idle = (*sounds)["idle"].get_or(std::string(""));
				sound.idleMinMs = static_cast<Uint32>(min) * 1000;
				sound.idleMaxMs = static_cast<Uint32>(max) * 1000;
				sound.loop = (*sounds)["loop"].get_or(std::string(""));
				sound.footsteps = (*sounds)["footsteps"].get_or(false);					
			}


			//Backpack
			sol::optional<sol::table> backpackTable = entity["components"]["backpack"];
			if (backpackTable != sol::nullopt) 
			{
				BackpackComponent backpack;
				backpack.capacity = (*backpackTable)["capacity"].get_or(0);

				sol::optional<sol::table> items = (*backpackTable)["items"];
				if (items) {
					for (const auto& [key, value] : *items) {
						if (value.get_type() != sol::type::string) {
							Logger::Warn("backpack: an entry is not an item id, skipped");
							continue;
						}
						std::string id = value.as<std::string>();

						if (itemDatabase.Find(id) == nullptr) {
							Logger::Warn("backpack: unknown item '" + id + "', skipped");
							continue;
						}

						if (backpack.IsFull()) {
							Logger::Warn("backpack: full (capacity " + std::to_string(backpack.capacity) + "), '" + id + "' not added");
							continue;
						}

						backpack.Add(id);

					}
				}

				newEntity.AddComponent<BackpackComponent>(backpack);
			}

			//Equipment
			sol::optional<sol::table> equipmentTable = entity["components"]["equipment"];
			if (equipmentTable != sol::nullopt) 
			{
				
				EquipmentComponent equipment;
				sol::optional<sol::table> slots = (*equipmentTable)["slots"];
				if (slots) {
					for (const auto& [key, value] : *slots) {
						if (value.get_type() != sol::type::table) {
							Logger::Warn("equipment: slot " + std::to_string(key.as<int>()) +
								" is not a table (write it as { name = \"...\", accepts = \"...\" }), skipped");
							continue;
						}
						sol::table slot = value.as<sol::table>();
						std::string name = slot["name"].get_or(std::string(""));
						std::string accepts = slot["accepts"].get_or(std::string(""));

						if (name.empty() || accepts.empty()) {
							Logger::Warn("equipment: slot " + std::to_string(key.as<int>()) +
								" needs both a name and accepts, skipped");
							continue;
						}

						if (equipment.FindSlot(name) != -1) {
							Logger::Warn("equipment: slot name '" + name + "' is used twice, skipped");
							continue;
						}
						
						//optional position in the equipment window, in grid cells
						int column = slot["column"].get_or(-1);
						int row = slot["row"].get_or(-1);

						if (column < 0 || row < 0) {
							if (column != -1 || row != -1) {
								Logger::Warn("equipment: slot '" + name + "' needs both column and row >= 0, placed automatically");
							}
							column = -1;
							row = -1;
						}
						else if (equipment.FindSlotAt(column, row) != -1) {
							Logger::Warn("equipment: slot '" + name + "' uses a cell another slot already has, placed automatically");
							column = -1;
							row = -1;
						}
						EquipmentSlot newSlot{ name, accepts, "" };
						newSlot.column = column;
						newSlot.row = row;
						equipment.slots.push_back(newSlot);
					}

					//slots without a Lua position go in a row under the lowest placed one, in Lua order
					int lowestRow = -1;
					for (const auto& s : equipment.slots) {
						lowestRow = std::max(lowestRow, s.row);
					}
					int nextColumn = 0;
					for (auto& slot : equipment.slots) {
						if (slot.column < 0) {
							slot.column = nextColumn++;
							slot.row = lowestRow + 1;
						}
					}
				}
				

				sol::optional<sol::table> equipped = (*equipmentTable)["equipped"];
				if (equipped) {
					for (const auto& [key, value] : *equipped) {
						
						if (key.get_type() != sol::type::string || value.get_type() != sol::type::string) {
							Logger::Warn("equipment: equipped entries must be written as SlotName = \"item_id\", skipped");
							continue;
						}

						std::string slotName = key.as<std::string>();
						std::string itemId = value.as<std::string>();

						int index = equipment.FindSlot(slotName);
						if (index == -1) {
							Logger::Warn("equipment: no slot named '" + slotName + "', '" + itemId + "' not equipped");
							continue;
						}
						const ItemDefinition* item = itemDatabase.Find(itemId);
						if (item == nullptr) {
							Logger::Warn("equipment: unknown item '" + itemId + "', slot '" + slotName + "' left empty");
							continue;
						}
						
						if (item->tag != equipment.slots[index].accepts) {
							Logger::Warn("equipment: " + itemId + " (tag '" + item->tag + "') can't go in slot '" + slotName + "', left empty");
							continue;
						}

						equipment.slots[index].itemId = itemId;
					}
				}
				
				for (const auto& s : equipment.slots) {
					if (s.accepts == "weapon" && s.itemId.empty()) {
						Logger::Warn("equipment: weapon slot '" + s.name + "' is empty, starts unarmed");
					}
				}

				for (const auto& [id, definition] : itemDatabase.All()) {
					bool accepted = false;
					for (const auto& s : equipment.slots) {
						if (s.accepts == definition.tag) {
							accepted = true;
							break;
						}
					}
					if (!accepted) {
						Logger::Warn("items: " + id + " (tag '" + definition.tag + "') can never be equipped");
					}
				}

				newEntity.AddComponent<EquipmentComponent>(equipment);

				if (newEntity.HasComponent<AttributesComponent>()) {
					InventoryRules::RebuildBonuses(newEntity.GetComponent<AttributesComponent>(), equipment, itemDatabase);
				}
			}


			//Loot
			sol::optional<sol::table> lootTable = entity["components"]["loot"];
			if (lootTable != sol::nullopt)
			{
				const std::string table = entity["components"]["loot"]["table"].get_or(std::string(""));
				if (lootDatabase.FindTable(table) == nullptr) {
					Logger::Warn("The loot table: " + table + " has unknown name or it is empty, skipped");
				}
				
				else {
					newEntity.AddComponent<LootDropComponent>(table);
				}
			}

			//KeyBoardControlled
			sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controlled"];
			if (keyboardControlled != sol::nullopt)
			{
				newEntity.AddComponent<KeyBoardControlledComponent>(
					glm::vec2(
						entity["components"]["keyboard_controlled"]["up_velocity"]["x"],
						entity["components"]["keyboard_controlled"]["up_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controlled"]["right_velocity"]["x"],
						entity["components"]["keyboard_controlled"]["right_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controlled"]["down_velocity"]["x"],
						entity["components"]["keyboard_controlled"]["down_velocity"]["y"]
					),
					glm::vec2(
						entity["components"]["keyboard_controlled"]["left_velocity"]["x"],
						entity["components"]["keyboard_controlled"]["left_velocity"]["y"]
					),
					entity["components"]["keyboard_controlled"]["diagnalMovement"]
				);
			}

			//Scripts
			sol::optional<sol::table> script = entity["components"]["on_update_script"];
			if (script != sol::nullopt)
			{
				sol::function func = entity["components"]["on_update_script"][0];
				newEntity.AddComponent<ScriptComponent>(func);
			}
		}
		i++;
	}

}
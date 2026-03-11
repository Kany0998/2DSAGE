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
#include "../Components/ProjectileComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/ScriptComponent.h"
#include <fstream>
#include <sol/sol.hpp>
#include <string>
#include "../Logger/Logger.h"

LevelLoader::LevelLoader()
{
	Logger::Log("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader()
{
	Logger::Log("LevelLoader destructor called!");
}

void LevelLoader::LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetStore>& assetStore, SDL_Renderer* renderer, int levelNumber)
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

		if (assetType == "texture")
		{
			assetStore->AddTexture(renderer, asset["id"], asset["file"]);
			Logger::Log("New texture assets loaded to the asset store, id: " + assetId);
		}

		if (assetType == "font")
		{
			assetStore->AddFont(assetId, asset["file"], asset["font_size"]);
			Logger::Log("New font assets loaded to the asset store, id: " + assetId);
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
	int layer = map["layer"];
	std::fstream mapFile;
	mapFile.open(mapFilePath);

	for (int y = 0; y < mapNumRows; y++)
	{
		for (int x = 0; x < mapNumCols; x++)
		{
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>(mapTextureAssetId, tileSize, tileSize, layer, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
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
				newEntity.AddComponent<TransformComponent>(
					glm::vec2(
						entity["components"]["transform"]["position"]["x"],
						entity["components"]["transform"]["position"]["y"]
					),
					glm::vec2(
						entity["components"]["transform"]["scale"]["x"].get_or(1.0),
						entity["components"]["transform"]["scale"]["y"].get_or(1.0)
					),
					entity["components"]["transform"]["rotation"].get_or(0.0)
					);
			}

			//RigidBody
			sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
			if (rigidbody != sol::nullopt)
			{
				newEntity.AddComponent<RigidBodyComponent>(
					glm::vec2(
						entity["components"]["rigidbody"]["velocity"]["x"],
						entity["components"]["rigidbody"]["velocity"]["y"]
					)
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
					static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
				);
			}
			
			//ProjectileEmitter
			sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
			if (projectileEmitter != sol::nullopt)
			{
				newEntity.AddComponent<ProjectileEmitterComponent>(
					glm::vec2(
						entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
						entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
					),
					static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1) * 1000),
					static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10) * 1000),
					static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
					entity["components"]["projectile_emitter"]["friendly"].get_or(false)
				);
			}

			//CameraFollow
			sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
			if (cameraFollow != sol::nullopt)
			{
				newEntity.AddComponent<CameraHollderComponent>();
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

	//Adding assets to asset Store
	/*assetStore->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assetStore->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
	assetStore->AddTexture(renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
	assetStore->AddTexture(renderer, "tilemap-image", "./assets/tilemaps/jungle.png");
	assetStore->AddTexture(renderer, "radar-image", "./assets/images/radar.png");
	assetStore->AddTexture(renderer, "bullet-image", "./assets/images/bullet.png");
	assetStore->AddTexture(renderer, "tree-image", "./assets/images/tree.png");
	assetStore->AddFont("charriot-font", "./assets/fonts/charriot.ttf", 14);


	//Load the titlemap
	int tileSize = 32;
	double tileScale = 4.0;
	int mapNumCols = 25;
	int mapNumRows = 20;
	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++)
	{
		for (int x = 0; x < mapNumCols; x++)
		{
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, SpriteComponent::LAYER_TILEMAP, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
	Game::mapWidth = mapNumCols * tileSize * tileScale;
	Game::mapHeight = mapNumRows * tileSize * tileScale;

	//entity creation example
	Entity chopper = registry->CreateEntity();
	chopper.Tag("player");
	chopper.AddComponent<TransformComponent>(glm::vec2(100.0, 100.0), glm::vec2(4.0, 4.0), 0.0);
	chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));// we dont have to put velocity here becuse we are cahnging it with keyboard input 
	//but for safty and requirenmsts of KeyBoardControlledComponent we put it here we start at inital speed 0
	chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, SpriteComponent::LAYER_PLAYER_IN_AIR);
	chopper.AddComponent<AnimationComponent>(2, 10, true);
	chopper.AddComponent<BoxColliderComponent>(32, 32);
	chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(350.0, 350.0), 0, 5000, 50, true);
	chopper.AddComponent<KeyBoardControlledComponent>(glm::vec2(0, -250), glm::vec2(250, 0), glm::vec2(0, 250), glm::vec2(-250, 0), !false);
	chopper.AddComponent<CameraHollderComponent>();
	chopper.AddComponent<HealthComponent>(100);

	Entity radar = registry->CreateEntity();
	radar.AddComponent<TransformComponent>(glm::vec2(Game::windowWidth - 74, 10), glm::vec2(1.0, 1.0), 0.0);
	radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	radar.AddComponent<SpriteComponent>("radar-image", 64, 64, SpriteComponent::LAYER_GUI, true);
	radar.AddComponent<AnimationComponent>(8, 1, true);

	Entity tank = registry->CreateEntity();
	tank.Group("enemies");
	tank.AddComponent<TransformComponent>(glm::vec2(1600.0, 225.0), glm::vec2(2.0, 2.0), 0.0);
	tank.AddComponent<RigidBodyComponent>(glm::vec2(50.0, 0.0));
	tank.AddComponent<SpriteComponent>("tank-image", 32, 32, SpriteComponent::LAYER_ENEMIES);
	tank.AddComponent<BoxColliderComponent>(32, 32);
	tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 100.0), 3000, 3000, 10, false);
	tank.AddComponent<HealthComponent>(100);

	Entity truck = registry->CreateEntity();
	truck.Group("enemies");
	truck.AddComponent<TransformComponent>(glm::vec2(250.0, 980.0), glm::vec2(3.0, 3.0), 0.0);
	truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	truck.AddComponent<SpriteComponent>("truck-image", 32, 32, SpriteComponent::LAYER_ENEMIES);
	truck.AddComponent<BoxColliderComponent>(32, 32);
	truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 1000, 3000, 10, false);
	truck.AddComponent<HealthComponent>(100);

	Entity treeA = registry->CreateEntity();
	treeA.Group("obstacles");
	treeA.AddComponent<TransformComponent>(glm::vec2(2000.0, 225.0), glm::vec2(2.0, 2.0), 0.0);
	treeA.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	treeA.AddComponent<SpriteComponent>("tree-image", 16, 32, SpriteComponent::LAYER_OBSTACLE);
	treeA.AddComponent<BoxColliderComponent>(16, 32);

	Entity treeB = registry->CreateEntity();
	treeB.Group("obstacles");
	treeB.AddComponent<TransformComponent>(glm::vec2(1500.0, 225.0), glm::vec2(2.0, 2.0), 0.0);
	treeB.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	treeB.AddComponent<SpriteComponent>("tree-image", 16, 32, SpriteComponent::LAYER_OBSTACLE);
	treeB.AddComponent<BoxColliderComponent>(16, 32);

	Entity label = registry->CreateEntity();
	SDL_Color white = { 255,255,255 };
	label.AddComponent<TextLabelComponent>(glm::vec2(400, 400), "Test Label String", "charriot-font", white, false);
	*/
}
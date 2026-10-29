#ifndef GAME_H
#define GAME_H
#include <SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include "../TileMap/TileMap.h"
#include "../Inventory/ItemDatabase.h"
#include "../Pathfinding/Pathfinder.h"
#include "../Inventory/InventoryUISettings.h"
#include "../Audio/AudioSettings.h"
#include "SimulationSettings.h"
#include "FlankingSettings.h"
#include "../Loot/LootDatabase.h"
#include "../Audio/AudioStats.h"
#include "../Abilities/AbilityDatabase.h"
#include "../Pathfinding/PathBenchmark.h"
#include "../Editor/EditorState.h"
#include "Controls.h"
#include <sol/sol.hpp>

const int FPS = 60;
const int frameDelay = 1000 / FPS;

// Forward-declared only: their Update() bodies reference Game::mapWidth/windowWidth
// etc., so their headers must be included from Game.cpp (after `class Game` is
// fully defined there), not from here.
class MovementSystem;
class RenderSystem;
class AnimationSystem;
class CollisionSystem;
class RenderCollisionSystem;
class DamageSystem;
class KeyboardControlSystem;
class CameraMovementSystem;
class ProjectileEmitSystem;
class ProjectileLifeCycleSystem;
class RenderTextSystem;
class RenderHealthBarSystem;
class HealthRegenerationSystem;
class RenderManaBarSystem;
class ManaRegenerationSystem;
class RenderGUISystem;
class ScriptSystem;
class SpecialAbilitySystem;
class ProgressionSystem;
class RenderExperienceBarSystem;
class AISystem;
class RenderTileMapSystem;
class LootSystem;
class AudioSystem;
class ActivationSystem;
class RenderEditorGUISystem;
class EditorSystem;

class Game
{
	private:
		bool isRunning;
		bool isDebug;
		bool editorMode;
		bool audioReady = false;     //false = no audio device, the game runs silently
		SDL_Window* window;
		SDL_Renderer* renderer;
		int millisecondsPreviousFrame = 0;
		SDL_Rect camera;
		Uint32 currentTick;

		double aiTimeSumMs = 0.0;
		double aiTimeMaxMs = 0.0;
		int aiTimeFrames = 0;
		double movementTimeSumMs = 0.0;
		double movementTimeMaxMs = 0.0;
		double collisionTimeSumMs = 0.0;
		double collisionTimeMaxMs = 0.0;

		sol::state lua;

		//Render-time measurement (debug mode only): summed over a batch of frames, then averaged
		double renderTimeSumMs = 0.0;
		int renderTimeFrames = 0;
		static constexpr int renderTimeBatch = 300;	//about 5 s at 60 FPS

		//entt-backed registry (component storage/sparse sets); see ECS.h
		std::unique_ptr<Registry> registry;

		std::unique_ptr<AssetStore> assetStore;
		std::unique_ptr<EventBus> eventBus;
		TileMap tileMap;
		ItemDatabase itemDatabase;
		LootDatabase lootDatabase;
		AbilityDatabase abilityDatabase;
		Controls controls;
		InventoryUISettings inventoryUI;
		Pathfinder pathfinder;
		Pathfinder debugPathfinder;		//debug view only - its weights never affect the enemies
		bool logPathStats = false;		//log the next debug search once (set by keys 1/2/3)
		std::string debugPathMode = "A*";
		AudioSettings audioSettings;
		SimulationSettings simulationSettings;
		FlankingSettings flankingSettings;
		std::vector<int> debugPath;
		AudioStats audioStats;
		PathBenchmarkSettings pathBenchmark;
		EditorState editorState;

		//Systems are owned directly by Game now instead of going through a
		//Registry-managed system map (entt has no notion of "systems" - they're
		//just plain objects that query the registry).
		std::unique_ptr<MovementSystem> movementSystem;
		std::unique_ptr<RenderSystem> renderSystem;
		std::unique_ptr<AnimationSystem> animationSystem;
		std::unique_ptr<CollisionSystem> collisionSystem;
		std::unique_ptr<RenderCollisionSystem> renderCollisionSystem;
		std::unique_ptr<DamageSystem> damageSystem;
		std::unique_ptr<KeyboardControlSystem> keyboardControlSystem;
		std::unique_ptr<CameraMovementSystem> cameraMovementSystem;
		std::unique_ptr<ProjectileEmitSystem> projectileEmitSystem;
		std::unique_ptr<ProjectileLifeCycleSystem> projectileLifeCycleSystem;
		std::unique_ptr<RenderTextSystem> renderTextSystem;
		std::unique_ptr<RenderHealthBarSystem> renderHealthBarSystem;
		std::unique_ptr<HealthRegenerationSystem> healthRegenerationSystem;
		std::unique_ptr<RenderManaBarSystem> renderManaBarSystem;
		std::unique_ptr<ManaRegenerationSystem> manaRegenerationSystem;
		std::unique_ptr<RenderGUISystem> renderGUISystem;
		std::unique_ptr<ScriptSystem> scriptSystem;
		std::unique_ptr<SpecialAbilitySystem> specialAbilitySystem;
		std::unique_ptr<ProgressionSystem> progressionSystem;
		std::unique_ptr<RenderExperienceBarSystem> renderExperienceBarSystem;
		std::unique_ptr<AISystem> aiSystem;
		std::unique_ptr<RenderTileMapSystem> renderTileMapSystem;
		std::unique_ptr<LootSystem> lootSystem;
		std::unique_ptr<AudioSystem> audioSystem;
		std::unique_ptr<ActivationSystem> activationSystem;
		std::unique_ptr<EditorSystem> editorSystem;
		std::unique_ptr<RenderEditorGUISystem> renderEditorGUISystem;


	public:
		Game();
		~Game();
		void Initialize();
		void Destroy();
		void Run();
		void Setup();
		void ProccessInput();
		void Update();
		void Render();

		static int windowWidth;
		static int windowHeight;
		static int mapWidth;
		static int mapHeight;
};

#endif

#ifndef GAME_H
#define GAME_H
#include <SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include "sol/sol.hpp"

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

class Game
{
	private:
		bool isRunning;
		bool isDebug;
		SDL_Window* window;
		SDL_Renderer* renderer;
		int millisecondsPreviousFrame = 0;
		SDL_Rect camera;
		Uint32 currentTick;

		sol::state lua;

		//entt-backed registry (component storage/sparse sets); see ECS.h
		std::unique_ptr<Registry> registry;

		std::unique_ptr<AssetStore> assetStore;
		std::unique_ptr<EventBus> eventBus;

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

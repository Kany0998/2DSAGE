#ifndef GAME_H
#define GAME_H
#include <SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include "sol/sol.hpp"

const int FPS = 60;
const int frameDelay = 1000 / FPS;

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

		std::unique_ptr<Registry> registry; //Registry * registry
		std::unique_ptr<AssetStore> assetStore;
		std::unique_ptr<EventBus> eventBus;


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

#include "Game.h"
#include "./LevelLoader.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderCollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifeCycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/HealthRegenerationSystem.h"
#include "../Systems/RenderManaBarSystem.h"
#include "../Systems/ManaRegenerationSystem.h"
#include "../Systems/RenderGUISystem.h"
#include "../Systems/ScriptSystem.h"
#include "../Systems/SpecialAbilitySystem.h"
#include "../Systems/ProgressionSystem.h"
#include "../Systems/RenderExperienceBarSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Events/MouseButtonPressedEvent.h"
#include <iostream>
#include <SDL.h>
#include <glm/glm.hpp>
#include <SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>


int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game()
{
	//TODO: implement
	isRunning = false;
	isDebug = false;
	registry = std::make_unique<Registry>();
	assetStore = std::make_unique<AssetStore>();
	eventBus = std::make_unique<EventBus>();
	Logger::Log("Game Constructor");
}

Game::~Game()
{
	Logger::Log("Game Destructor");
}

void Game::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		Logger::Err("Error initializing SDL.");
		return;
	}

	if(TTF_Init() != 0)
	{
		Logger::Err("Error initializing SDL TTF.");
		return;
	}
	//Create window
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	windowWidth = displayMode.w; //800
	windowHeight = displayMode.h;//600

	window = SDL_CreateWindow(
		"2DSAGE",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) 
	{
		Logger::Err("Error creating SDL window.");
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		Logger::Err("Error creating SDL renderer.");
		return;
	}
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	//Initialize ImGui
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);

	//Initialize camera view with entire screen area
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;
	isRunning = true;
}

void Game::ProccessInput()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		//ImGui SDL input
		ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
		ImGuiIO& io = ImGui::GetIO();

		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
		io.MousePos = ImVec2(mouseX, mouseY);
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

		//Handle core SDl events
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if(sdlEvent.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}

			if (sdlEvent.key.keysym.sym == SDLK_x)
			{
				isDebug = !isDebug;
			}
			eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
			break;

		case SDL_MOUSEBUTTONDOWN:
			//Don't let a click meant for the ImGui debug panel also fire a projectile
			if (!io.WantCaptureMouse)
			{
				glm::vec2 worldPosition(
					sdlEvent.button.x + camera.x,
					sdlEvent.button.y + camera.y
				);
				eventBus->EmitEvent<MouseButtonPressedEvent>(sdlEvent.button.button, worldPosition);
			}
			break;

		}
	}
}


void Game::Setup()
{
	//Construct the systems that need to be processed in the game loop
	movementSystem = std::make_unique<MovementSystem>();
	renderSystem = std::make_unique<RenderSystem>();
	animationSystem = std::make_unique<AnimationSystem>();
	collisionSystem = std::make_unique<CollisionSystem>();
	renderCollisionSystem = std::make_unique<RenderCollisionSystem>();
	damageSystem = std::make_unique<DamageSystem>(eventBus);
	keyboardControlSystem = std::make_unique<KeyboardControlSystem>();
	cameraMovementSystem = std::make_unique<CameraMovementSystem>();
	projectileEmitSystem = std::make_unique<ProjectileEmitSystem>(*registry, camera);
	projectileLifeCycleSystem = std::make_unique<ProjectileLifeCycleSystem>();
	renderTextSystem = std::make_unique<RenderTextSystem>();
	renderHealthBarSystem = std::make_unique<RenderHealthBarSystem>();
	healthRegenerationSystem = std::make_unique<HealthRegenerationSystem>();
	renderManaBarSystem = std::make_unique<RenderManaBarSystem>();
	manaRegenerationSystem = std::make_unique<ManaRegenerationSystem>();
	renderGUISystem = std::make_unique<RenderGUISystem>();
	scriptSystem = std::make_unique<ScriptSystem>();
	specialAbilitySystem = std::make_unique<SpecialAbilitySystem>(*registry, camera);
	progressionSystem = std::make_unique<ProgressionSystem>();
	renderExperienceBarSystem = std::make_unique<RenderExperienceBarSystem>();

	//create bindings between c++ and lua
	scriptSystem->CreateLuaBindings(lua);

	//Load first level
	LevelLoader loader;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
	loader.LoadLevel(lua, registry, assetStore, tileMap, renderer, 1);
}

void Game::Update()
{
	// initial position and velocity // Fps
	int timeToWait = frameDelay - (SDL_GetTicks() - millisecondsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= frameDelay)
	{
		SDL_Delay(timeToWait);
	}

	//differnace in ticks since last frame conveted to seconds
	double deltaTime = (SDL_GetTicks() - millisecondsPreviousFrame) / 1000.0;
	//Time stored
	millisecondsPreviousFrame = SDL_GetTicks();

	//Reset all event Handlers for the current frame
	eventBus->Reset();

	//perform subscribtion to events for all the systems
	movementSystem->SubscribeToEvents(eventBus);
	damageSystem->SubscribeToEvents();
	keyboardControlSystem->SubscribeToEvents(eventBus);
	//ProjectileEmitSystem no longer subscribes to anything: it polls the mouse in its
	//own Update() so holding the button keeps firing, which a one-shot SDL button-down
	//event can't express.
	specialAbilitySystem->SubscribeToEvents(eventBus);
	progressionSystem->SubscribeToEvents(eventBus);
	//Update the registry to process the entites that are waiting to boe created/deleted
	registry->Update();

	// invoke all the systems to update
	keyboardControlSystem->Update(*registry);
	movementSystem->Update(*registry, deltaTime, tileMap);
	animationSystem->Update(*registry);
	collisionSystem->Update(*registry, eventBus);
	cameraMovementSystem->Update(*registry, camera);
	projectileEmitSystem->Update();
	projectileLifeCycleSystem->Update(*registry);
	healthRegenerationSystem->Update(*registry, deltaTime);
	manaRegenerationSystem->Update(*registry, deltaTime);
	progressionSystem->Update(*registry);
	scriptSystem->Update(*registry, deltaTime, SDL_GetTicks());

}

void Game::Render()
{
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	//Invoke all the systems that need to render
	renderSystem->Update(*registry, renderer, assetStore, camera);
	renderTextSystem->Update(*registry, renderer, assetStore, camera);
	renderHealthBarSystem->Update(*registry, renderer, assetStore, camera);
	renderManaBarSystem->Update(*registry, renderer, assetStore, camera);
	renderExperienceBarSystem->Update(*registry, renderer, assetStore, camera);
	if (isDebug)
	{
		renderCollisionSystem->Update(*registry, renderer, camera);

		
	}
	renderGUISystem->Update(*registry, camera, isDebug, tileMap);

	SDL_RenderPresent(renderer);
}

void Game::Run()
{
	Setup();
	while(isRunning)
	{
		ProccessInput();
		Update();
		Render();
	}
}

void Game::Destroy()
{
	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();

	//Free every SDL_Texture/TTF_Font now, while the renderer and SDL_ttf are
	//still alive - AssetStore otherwise wouldn't be destroyed until Game's own
	//destructor runs, which happens after SDL_Quit()/TTF_Quit() below.
	assetStore.reset();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}
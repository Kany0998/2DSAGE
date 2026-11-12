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
#include "../Systems/AISystem.h"
#include "../Systems/RenderTileMapSystem.h"
#include "../Systems/LootSystem.h"
#include "../Systems/AudioSystem.h"
#include "../Systems/ActivationSystem.h"
#include "../Editor/RenderEditorGUISystem.h"
#include "../Editor/EditorSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Events/MouseButtonPressedEvent.h"
#include "../TileMap/MovementType.h"
#include "../Pathfinding/Pathfinder.h"
#include "../Audio/LevelAmbience.h"
#include <iostream>
#include <SDL.h>
#include <glm/glm.hpp>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>


int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game()
{
	isRunning = false;
	isDebug = false;
	editorMode = false;
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

	//44.1 kHz, default format, stereo, 2048-sample buffer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
		Logger::Warn(std::string("Audio device could not be opened, playing without sound: ") + Mix_GetError());
	}
	
	else {
		audioReady = true;
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
	// The SDL backend only needs the window here; rendering goes through SDL_Renderer + ImGuiSDL.
	// InitForOpenGL has no platform assert (InitForD3D asserts outside Windows); the GL context is unused.
	ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
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
		switch (sdlEvent.type){
			case SDL_QUIT:
				isRunning = false;
				break;

			case SDL_KEYDOWN:

				if (io.WantCaptureKeyboard) {
					break;                       // ImGui is typing: no game shortcuts
				}

				if(sdlEvent.key.keysym.sym == SDLK_ESCAPE){
					isRunning = false;
				}

				if (sdlEvent.key.keysym.sym == SDLK_x){
					isDebug = !isDebug;
				}

				if (sdlEvent.key.keysym.sym == controls.editor) {
					editorMode = !editorMode;

					if (!editorMode) {
						Game::mapWidth = static_cast<int>(tileMap.Cols() * tileMap.TileWorldSize());
						Game::mapHeight = static_cast<int>(tileMap.Rows() * tileMap.TileWorldSize());
						for (auto rawEntity : registry->Raw().view <AIComponent>()) {
							Entity entity(rawEntity, registry.get());
							auto& enemyAI = entity.GetComponent<AIComponent>();
							EnterAIState(enemyAI, enemyAI.state);
						}
					}
					

					
					if (audioReady) {
						//get channels count
						int channelCount = Mix_AllocateChannels(-1);
						// int channel = 1 make everything silent except ambience loop
						// int channel = 0 make everything silent
						for (int channel = 1; channel < channelCount; ++channel) {
							if (editorMode) {
								Mix_Pause(channel);
							}
							else {
								Mix_Resume(channel);
							}
						}
					}

				}

				if (sdlEvent.key.keysym.sym == SDLK_r && gameState != GameState::Playing) {
					restartRequested = true;
				}

				if (sdlEvent.key.keysym.sym == SDLK_p) {
					if (gameState == GameState::Playing) {
						gameState = GameState::Paused;
					}
					else if (gameState == GameState::Paused) {
						gameState = GameState::Playing;
					}
				}

				if (isDebug) {
					if (sdlEvent.key.keysym.sym == SDLK_1){
						debugPathfinder.gWeight = 1.0;
						debugPathfinder.hWeight = 0.0;
						debugPathMode = "Dijkstra";
						logPathStats = true;
					}

					if (sdlEvent.key.keysym.sym == SDLK_2){
						debugPathfinder.gWeight = 1.0;
						debugPathfinder.hWeight = 1.0;
						debugPathMode = "A*";
						logPathStats = true;
					}

					if (sdlEvent.key.keysym.sym == SDLK_3){
						debugPathfinder.gWeight = 0.0;
						debugPathfinder.hWeight = 1.0;
						debugPathMode = "Greedy";
						logPathStats = true;
					}

					if (sdlEvent.key.keysym.sym == SDLK_4){
						debugPathfinder.gWeight = 1.0;
						debugPathfinder.hWeight = 1.0 / std::max(1.0, tileMap.MaxSpeedMultiplier());
						debugPathMode = "A* admissible";
						logPathStats = true;
					}

					if (sdlEvent.key.keysym.sym == SDLK_F9){
						if (pathBenchmark.routes.empty()) {
							Logger::Log("There are no routes");
						}
						else {
							RunPathBenchmark(pathBenchmark, tileMap);
						}
					}
				}


				eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym, sdlEvent.key.repeat != 0);
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
	projectileEmitSystem = std::make_unique<ProjectileEmitSystem>(*registry, camera, tileMap, itemDatabase, *assetStore, eventBus);
	projectileLifeCycleSystem = std::make_unique<ProjectileLifeCycleSystem>();
	renderTextSystem = std::make_unique<RenderTextSystem>();
	renderHealthBarSystem = std::make_unique<RenderHealthBarSystem>();
	healthRegenerationSystem = std::make_unique<HealthRegenerationSystem>();
	renderManaBarSystem = std::make_unique<RenderManaBarSystem>();
	manaRegenerationSystem = std::make_unique<ManaRegenerationSystem>();
	renderGUISystem = std::make_unique<RenderGUISystem>(controls, inventoryUI, itemDatabase, lootDatabase, audioSettings, eventBus, audioStats, *assetStore);
	scriptSystem = std::make_unique<ScriptSystem>();
	specialAbilitySystem = std::make_unique<SpecialAbilitySystem>(*registry, camera, itemDatabase, abilityDatabase, eventBus);
	progressionSystem = std::make_unique<ProgressionSystem>(eventBus);
	renderExperienceBarSystem = std::make_unique<RenderExperienceBarSystem>();
	aiSystem = std::make_unique<AISystem>();
	renderTileMapSystem = std::make_unique<RenderTileMapSystem>();
	lootSystem = std::make_unique<LootSystem>(*registry, lootDatabase, itemDatabase, *assetStore, eventBus);
	activationSystem = std::make_unique<ActivationSystem>();
	editorSystem = std::make_unique<EditorSystem>(editorState);
	renderEditorGUISystem = std::make_unique<RenderEditorGUISystem>(editorState, *assetStore);
	

	//create bindings between c++ and lua
	scriptSystem->CreateLuaBindings(lua, *registry, *assetStore, tileMap);

	//Load first level
	LevelLoader loader;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table);
	itemDatabase.Load(lua, "./assets/scripts/items.lua");
	lootDatabase.Load(lua, "./assets/scripts/loot.lua", itemDatabase);
	abilityDatabase.Load(lua, "./assets/scripts/abilities.lua", itemDatabase);
	loader.LoadLevel(lua, registry, assetStore, tileMap, itemDatabase, lootDatabase, renderer, 1);
	levelHadEnemies = !registry->GetEntitiesByGroup("enemies").empty();		//check if enemies exist
	pathBenchmark = PathBenchmarkSettings::Load(lua, tileMap);
	Logger::Log("Routes size: " + std::to_string(pathBenchmark.routes.size()));
	Logger::Log("Number of runs: " + std::to_string(pathBenchmark.runs));
	controls = Controls::Load(lua);
	inventoryUI = InventoryUISettings::Load(lua);
	simulationSettings = SimulationSettings::Load(lua);
	flankingSettings = FlankingSettings::Load(lua);
	audioSettings = AudioSettings::Load(lua);
	audioSystem = std::make_unique<AudioSystem>(*registry, *assetStore, audioSettings, camera, tileMap, lootDatabase, eventBus, audioStats, audioReady);
	audioSystem->PlayMusic(lua["Level"]["music"].get_or(std::string("")));
	audioSystem->StartAmbience(LevelAmbience::Load(lua));
}


void Game::RestartLevel() {
	// clear everything from previous level
	for (auto [handle] : registry->Raw().storage<entt::entity>().each()) {
		Entity(handle, registry.get()).Kill();
	}
	registry->Update();

	lootSystem->ClearPendingDrops();

	//assets, loadLevel contains all of them so we need to clear it also we halts audio
	assetStore->ClearAssets();

	//load level the same as in Setup()
	LevelLoader loader;
	loader.LoadLevel(lua, registry, assetStore, tileMap, itemDatabase, lootDatabase, renderer, 1);
	levelHadEnemies = !registry->GetEntitiesByGroup("enemies").empty();

	audioSystem->PlayMusic(lua["Level"]["music"].get_or(std::string("")));
	audioSystem->StartAmbience(LevelAmbience::Load(lua));

	//back to playing
	editorMode = false;
	gameState = GameState::Playing;
}

void Game::Update()
{

	if (restartRequested) {
		restartRequested = false;
		RestartLevel();
	}
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
	damageSystem->SubscribeToEvents();
	keyboardControlSystem->SubscribeToEvents(eventBus);

	// must stay after keyboardControlSystem->Update: the keyboard writes the walk row every frame,
	// and the animation system overwrites it while an attack pose or the facing lock is active
	animationSystem->SubscribeToEvents(eventBus);
	//ProjectileEmitSystem no longer subscribes to anything: it polls the mouse in its
	//own Update() so holding the button keeps firing, which a one-shot SDL button-down
	//event can't express.
	specialAbilitySystem->SubscribeToEvents(eventBus);
	renderGUISystem->SubscribeToEvents();
	progressionSystem->SubscribeToEvents();
	lootSystem->SubscribeToEvents();
	audioSystem->SubscribeToEvents();
	//Update the registry to process the entites that are waiting to boe created/deleted
	registry->Update();


	//set the game (important placment if we place this above registry->Update() we can have memory leak since enemies killed in last frame werent removed)
	if (gameState == GameState::Playing) {
		if (!registry->HasEntityWithTag("player")) {
			gameState = GameState::Dead;
		}

		else if (levelHadEnemies && registry->GetEntitiesByGroup("enemies").empty()) {
			gameState = GameState::Victory;
		}
	}

	if (gameState != GameState::Playing) {
		return;
	}

	//Editor mode toggke
	if (editorMode) {
		editorSystem->Update(deltaTime, camera, tileMap);
		return;
	}

	// invoke all the systems to update
	lootSystem->Update();
	audioSystem->Update();
	keyboardControlSystem->Update(*registry);

	activationSystem->Update(*registry, deltaTime, simulationSettings, tileMap);

	const Uint64 aiStart = SDL_GetPerformanceCounter();
	aiSystem->Update(*registry, deltaTime, tileMap, pathfinder, flankingSettings);
	const double aiMs = (SDL_GetPerformanceCounter() - aiStart) * 1000.0 / SDL_GetPerformanceFrequency();
	Uint64 movementPerformanceStart = SDL_GetPerformanceCounter();
	movementSystem->Update(*registry, deltaTime, tileMap);
	const double movementMsEnd = ((SDL_GetPerformanceCounter() - movementPerformanceStart) * 1000.0 / SDL_GetPerformanceFrequency());

	animationSystem->Update(*registry);
	Uint64 colPerformance = SDL_GetPerformanceCounter();
	collisionSystem->Update(*registry, eventBus);
	const double collisionMs = ((SDL_GetPerformanceCounter() - colPerformance) * 1000.0 / SDL_GetPerformanceFrequency());

	if (isDebug) {
		aiTimeSumMs += aiMs;
		movementTimeSumMs += movementMsEnd;
		collisionTimeSumMs += collisionMs;
		aiTimeMaxMs = std::max(aiTimeMaxMs, aiMs);
		collisionTimeMaxMs = std::max(collisionTimeMaxMs, collisionMs);
		movementTimeMaxMs = std::max(movementTimeMaxMs, movementMsEnd);
		aiTimeFrames++;

		if (aiTimeFrames == renderTimeBatch) {
			Logger::Log("AI: avg: " + std::to_string(aiTimeSumMs / aiTimeFrames) + " ms/frame, max: " + std::to_string(aiTimeMaxMs) +
				" ms, searches: " + std::to_string(aiSystem->searchesDone) + ", deferred: " + std::to_string(aiSystem->searchesDeferred)
				+ ", pathfinder: " + std::to_string(aiSystem->pathfinderMS) + " ms, enemies: " + std::to_string(registry->Raw().view<AIComponent>().size())
				+ ", active: " + std::to_string(registry->Raw().view<ActiveComponent>().size()) + ", longest wait: " + std::to_string(aiSystem->longestWaitSeconds) +" s");
			Logger::Log("Movement performance: " + std::to_string(movementTimeSumMs / aiTimeFrames) + " ms/frame, max: "+ std::to_string(movementTimeMaxMs) );
			Logger::Log("Collision performance: " + std::to_string(collisionTimeSumMs / aiTimeFrames) + " ms/frame, max: "+std::to_string(collisionTimeMaxMs) +
				", pairs tested: " +std::to_string(collisionSystem->pairsTested) + ", events fired: " + std::to_string(collisionSystem->collisionsEvents));
			aiTimeSumMs = 0.0;
			aiTimeMaxMs = 0.0;
			collisionTimeSumMs = 0.0;
			collisionTimeMaxMs = 0.0;
			movementTimeSumMs = 0.0;
			movementTimeMaxMs = 0.0;
			aiTimeFrames = 0;
			aiSystem->searchesDone = 0;
			aiSystem->searchesDeferred = 0;
			aiSystem->pathfinderMS = 0;
			aiSystem->longestWaitSeconds = 0.0;
		}
	}
	else {
		aiTimeSumMs = 0.0;
		aiTimeMaxMs = 0.0;
		aiTimeFrames = 0;
		aiSystem->searchesDone = 0;
		aiSystem->searchesDeferred = 0;
		aiSystem->pathfinderMS = 0;
		collisionTimeSumMs = 0.0;
		collisionTimeMaxMs = 0.0;
		movementTimeSumMs = 0.0;
		movementTimeMaxMs = 0.0;
		aiSystem->longestWaitSeconds = 0.0;
	}

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

	//In debug mode the map and sprite rendering is timed and averaged over a batch of frames -
	//the measurement behind the tilemap refactor. FPS is capped, so the work itself is timed
	const Uint64 renderStart = isDebug ? SDL_GetPerformanceCounter() : 0;
	renderTileMapSystem->Update(renderer, assetStore, camera, tileMap);
	renderSystem->Update(*registry, renderer, assetStore, camera);

	if (isDebug) {
		const Uint64 renderEnd = SDL_GetPerformanceCounter();
		renderTimeSumMs += (renderEnd - renderStart) * 1000.0 / SDL_GetPerformanceFrequency();
		renderTimeFrames++;

		if (renderTimeFrames == renderTimeBatch) {
			const size_t entityCount = registry->Raw().view<TransformComponent>().size();
			Logger::Log("Render time (tilemap + sprites): " + std::to_string(renderTimeSumMs / renderTimeFrames) +
				" ms/frame over " + std::to_string(renderTimeFrames) +
				" frames, entities with a transform: " + std::to_string(entityCount));

			renderTimeSumMs = 0.0;
			renderTimeFrames = 0;
		}
	}
	else {
		//A batch half-filled before debug mode was switched off would otherwise be mixed into the next one
		renderTimeSumMs = 0.0;
		renderTimeFrames = 0;
	}

	renderTextSystem->Update(*registry, renderer, assetStore, camera);
	renderHealthBarSystem->Update(*registry, renderer, assetStore, camera);
	renderManaBarSystem->Update(*registry, renderer, assetStore, camera);
	renderExperienceBarSystem->Update(*registry, renderer, assetStore, camera);
	if (isDebug)
	{
		renderCollisionSystem->Update(*registry, renderer, camera);

		
	}

	//pathfinding debug view: search from the first enemy to the player and draw the result
	if (isDebug && registry->HasEntityWithTag("player")) {
		std::vector<Entity> enemies = registry->GetEntitiesByGroup("enemies");

		if (!enemies.empty()) {
			Entity player = registry->GetEntityByTag("player");
			Entity enemy = enemies[0];

			//Centre of the sprite, the same point MovementSystem tests against the tilemap
			const auto& pTransform = player.GetComponent<TransformComponent>();
			const auto& pSprite = player.GetComponent<SpriteComponent>();
			double pX = pTransform.position.x + pSprite.width * pTransform.scale.x / 2.0;
			double pY = pTransform.position.y + pSprite.height * pTransform.scale.y / 2.0;

			const auto& eTransform = enemy.GetComponent<TransformComponent>();
			const auto& eSprite = enemy.GetComponent<SpriteComponent>();
			double eX = eTransform.position.x + eSprite.width * eTransform.scale.x / 2.0;
			double eY = eTransform.position.y + eSprite.height * eTransform.scale.y / 2.0;

			int startCol = tileMap.colAt(eX), startRow = tileMap.rowAt(eY);
			int goalCol = tileMap.colAt(pX), goalRow = tileMap.rowAt(pY);

			double tileWorldSize = tileMap.TileWorldSize();

			SearchBounds bounds;
			if (enemy.HasComponent<AIComponent>()) {
				bounds = BoundsForEnemy(enemy.GetComponent<AIComponent>(), tileMap);
			}
			
			
			debugPathfinder.FindPath(tileMap, startCol, startRow, goalCol, goalRow, MovementType_Ground, debugPath, bounds);
			const auto& lastStats = debugPathfinder.LastStats();

			if (logPathStats) {
				Logger::Log("DebugPathMode: " + debugPathMode);
				Logger::Log("PathCost: " + std::to_string(lastStats.pathCost));
				Logger::Log("NodesExpanded: " + std::to_string(lastStats.nodesExpanded));
				Logger::Log("NodesPushed: " + std::to_string(lastStats.nodesPushed));
				Logger::Log("SearchMs: " + std::to_string(lastStats.searchMs));
				logPathStats = false;
			}

			int firstCol = std::max(0, static_cast<int>(std::floor(camera.x / tileWorldSize)));
			int lastCol = std::min(tileMap.Cols() - 1, static_cast<int>(std::floor((camera.x + camera.w) / tileWorldSize)));

			int firstRow = std::max(0, static_cast<int>(std::floor(camera.y / tileWorldSize)));
			int lastRow = std::min(tileMap.Rows() - 1, static_cast<int>(std::floor((camera.y + camera.h) / tileWorldSize)));

			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

			for (int row = firstRow; row <= lastRow; ++row) {
				for (int col = firstCol; col <= lastCol; ++col) {
					const int state = debugPathfinder.TileStateAt(tileMap.Index(col, row));

					if (state == Pathfinder::exploredTile) {
						SDL_SetRenderDrawColor(renderer, 59, 185, 246, 90);
					}
					else if (state == Pathfinder::frontierTile) {
						SDL_SetRenderDrawColor(renderer, 224, 168, 0, 110);
					}
					else {
						continue;
					}
					SDL_Rect cellRect = {
						static_cast<int>(col * tileWorldSize - camera.x + 1),
						static_cast<int>(row * tileWorldSize - camera.y + 1),
						static_cast<int>(tileWorldSize - 2),
						static_cast<int>(tileWorldSize - 2)
					};
					SDL_RenderFillRect(renderer, &cellRect);
				}
			}
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			
			SDL_SetRenderDrawColor(renderer, 155, 21, 21, 255);

			
			int pathSizeGrid = 20;

			//The line starts where the enemy actually is, the same anchor the smoothing used
			double previousScreenX = eX - camera.x;
			double previousScreenY = eY - camera.y;

			for (const int index : debugPath) {
				int col = tileMap.IndexToCol(index);
				int row = tileMap.IndexToRow(index);

				double screenCenterX = col * tileWorldSize + tileWorldSize / 2 - camera.x;
				double screenCenterY = row * tileWorldSize + tileWorldSize / 2 - camera.y;

				SDL_RenderDrawLine(renderer,
					static_cast<int>(previousScreenX),
					static_cast<int>(previousScreenY),
					static_cast<int>(screenCenterX),
					static_cast<int>(screenCenterY)
				);

				SDL_Rect pathRect = {
				static_cast<int>(screenCenterX - pathSizeGrid/2),
				static_cast<int>(screenCenterY - pathSizeGrid/2),
				static_cast<int>(pathSizeGrid),
				static_cast<int>(pathSizeGrid)
				};

				SDL_RenderFillRect(renderer, &pathRect);

				previousScreenX = screenCenterX;
				previousScreenY = screenCenterY;
			}
			
		}
	}
	//Create gui frames
	ImGui::NewFrame();
	if (editorMode) {
		renderEditorGUISystem->Update(camera, tileMap);
	}
	else {
		renderGUISystem->Update(*registry, camera, isDebug, tileMap);
	}

	if (gameState != GameState::Playing) {
		const char* title = "VICTORY";
		const char* hint = "Press R to restart";
		if (gameState == GameState::Dead) {
			title = "YOU DIED";
		}
		else if (gameState == GameState::Paused) {
			title = "PAUSED";
			hint = "P to unpause, R to restart";
		}

		ImGui::SetNextWindowPos(ImVec2(windowWidth / 2.0f, windowHeight / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::Begin("##endscreen", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

		ImGui::SetWindowFontScale(3.0f);
		ImGui::Text("%s", title);

		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("%s", hint);

		ImGui::End();
	}

	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());

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
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	//Free every SDL_Texture/TTF_Font now, while the renderer and SDL_ttf are
	//still alive - AssetStore otherwise wouldn't be destroyed until Game's own
	//destructor runs, which happens after SDL_Quit()/TTF_Quit() below.
	assetStore.reset();

	//If sound was played we destory it at the end
	if (audioReady) {
		Mix_CloseAudio();
	}

	Mix_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}
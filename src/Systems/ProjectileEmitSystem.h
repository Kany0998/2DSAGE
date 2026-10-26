#ifndef PROJECTILEEMITSYSTEM_H
#define PROJECTILEEMITSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/CameraHollderComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Components/EquipmentComponent.h"
#include "../Components/AIComponent.h"
#include "../Components/ActiveComponent.h"
#include "../Utils/EntityGeometry.h"
#include "../Utils/Projectile.h"
#include "../TileMap/TileMap.h"
#include "../Inventory/ItemDatabase.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include "../Events/ProjectileFiredEvent.h"
#include <SDL.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>

struct ProjectileProfile {
	std::string texture = "bullet-texture";
	std::string shotSound = "";
	int minDamage = 1, maxDamage = 1;
	float speed = 0.0f;
	float shotsPerSecond = 1.0f;   //player only; enemies keep repeatRate
	int lifetimeMs = 10000;
};


// Two kinds of emitter live here:
//  - the player (CameraHollderComponent): fires while the left mouse button is held,
//    toward the cursor. Its bullet comes from the equipped weapon, otherwise from its
//    own emitter (unarmed); the fire rate is the weapon's, scaled by dexterity.
//  - everything else (enemies): fires on the fixed repeatRate timer loaded from Lua,
//    with the bullet from its emitter. Enemies have no dexterity, so their rate is left alone.
class ProjectileEmitSystem{
	public:

		ProjectileEmitSystem(Registry& registry, const SDL_Rect& camera, const TileMap& tileMap, const ItemDatabase& itemDatabase, AssetStore& assetStore, std::unique_ptr<EventBus>& eventBus)
			: registry(registry), camera(camera), tileMap(tileMap), itemDatabase(itemDatabase), assetStore(assetStore), eventBus(eventBus) {}

		void Update(){
			// Snapshot first: emitting a projectile adds a TransformComponent to a
			// brand-new entity, which can grow/reallocate that component's pool.
			// Doing that while still iterating a live view over TransformComponent is
			// unsafe (can invalidate the iteration mid-loop) - materializing the
			// candidates first, like CollisionSystem does, avoids that entirely.
			std::vector<Entity> emitters;
			for (auto rawEntity : registry.Raw().view<ProjectileEmitterComponent, TransformComponent>()){
				emitters.emplace_back(rawEntity, &registry);
			}

			for (auto entity : emitters){
				auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
				//deliberately a copy, not a reference: the pool it lives in can
				//reallocate while the projectile below is being created
				const auto transform = entity.GetComponent<TransformComponent>();

				if (entity.HasComponent<CameraHollderComponent>()){
					UpdateMouseAimedEmitter(entity, projectileEmitter, transform);
				}
				else{
					UpdateAutomaticEmitter(entity, projectileEmitter, transform);
				}
			}
		}

	private:
		//Player shooting: fires for as long as the left button is held down, aimed at
		//wherever the cursor currently is. repeatRate is ignored here on purpose - the
		//rate comes from the profile and dexterity (see GetShotIntervalMs).
		void UpdateMouseAimedEmitter(Entity entity, ProjectileEmitterComponent& projectileEmitter, const TransformComponent& transform){
			//don't shoot through an ImGui window - the same guard Game::ProccessInput() applies
			//to the click event. ImGui runs a frame unconditionally now, so WantCaptureMouse is
			//always current and needs no debug-mode qualifier.
			if (ImGui::GetIO().WantCaptureMouse){
				return;
			}

			int mouseX, mouseY;
			const Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
			if (!(mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))){
				return;
			}
			ProjectileProfile profile = GetProfile(entity, projectileEmitter);

			if (SDL_GetTicks() - projectileEmitter.lastEmittedTime < GetShotIntervalMs(entity, profile)){
				return;
			}

			const glm::vec2 projectilePosition = GetMuzzlePosition(entity, transform);

			//mouse position is screen-space - camera offset turns it into the world-space
			//point the shot should travel toward
			const glm::vec2 aimTarget(mouseX + camera.x, mouseY + camera.y);
			glm::vec2 aimDirection = aimTarget - projectilePosition;
			const float distance = glm::length(aimDirection);
			if (distance < 0.0001f){
				return; //cursor sits right on top of the shooter - no direction to aim in
			}
			aimDirection /= distance;


			const float speed = profile.speed;

			EmitProjectile(entity, projectilePosition, aimDirection * speed, profile);
			projectileEmitter.lastEmittedTime = SDL_GetTicks();
		}

		void UpdateAutomaticEmitter(Entity entity, ProjectileEmitterComponent& projectileEmitter, const TransformComponent& transform){

			//Only entities ActivationSystem tagged as active may fire; it tags group "enemies" only,
			//so an automatic shooter outside that group never shoots
			if (!entity.HasComponent<ActiveComponent>()) {
				return;
			}

			const glm::vec2 projectilePosition = GetMuzzlePosition(entity, transform);
			ProjectileProfile profile = GetProfile(entity, projectileEmitter);
			//Default: the fixed velocity authored in Lua, for shooters with no AI
			glm::vec2 shotVelocity = projectileEmitter.projectileVelocity;
			
			if (projectileEmitter.aimAtPlayer) {

				//An AI shooter only fires while engaged; a turret has no AI and always may
				if (entity.HasComponent<AIComponent>() && entity.GetComponent<AIComponent>().state != AIState::Chase) {
					return;
				}

				if (!registry.HasEntityWithTag("player")) {
					return;
				}

				Entity player = registry.GetEntityByTag("player");
				if (!player.IsAlive()) {
					return;
				}
				const auto& playerTransform = player.GetComponent<TransformComponent>();
				const auto& playerSprite = player.GetComponent<SpriteComponent>();

				const glm::vec2 playerCenter = EntityCenter(playerTransform, playerSprite);

				const double dx = playerCenter.x - projectilePosition.x;
				const double dy = playerCenter.y - projectilePosition.y;
				const double distanceSquared = dx * dx + dy * dy;

				if (distanceSquared > projectileEmitter.attackRange * projectileEmitter.attackRange) {
					return;
				}
				const double length = std::sqrt(distanceSquared);

				if (length < 0.0001) {
					return;
				}

				const double speed = glm::length(projectileEmitter.projectileVelocity);
				shotVelocity = glm::vec2(dx / length * speed, dy / length * speed);

			}

			if (projectileEmitter.repeatRate == 0){
				return;
			}

			if (SDL_GetTicks() - projectileEmitter.lastEmittedTime <= projectileEmitter.repeatRate){
				return;
			}

			
			EmitProjectile(entity, projectilePosition, shotVelocity, profile);
			projectileEmitter.lastEmittedTime = SDL_GetTicks();
			
		}

		//Milliseconds the shooter has to wait between shots: the profile's shots per second
		//(weapon, or the unarmed base), multiplied by 1 + dexterity / DexterityPerStep.
		//All of it stays in float - integer division here would collapse whole ranges of
		//dexterity into an identical rate.
		int GetShotIntervalMs(Entity entity, const ProjectileProfile& profile){
			float shotsPerSecond = profile.shotsPerSecond;
			if (entity.HasComponent<AttributesComponent>()){
				const auto& attributes = entity.GetComponent<AttributesComponent>();
				shotsPerSecond *= (1.0f + attributes.Effective(Skill_Dexterity) / DexterityPerStep);
			}

			//guards against a zero/negative rate if dexterity ever goes negative
			if (shotsPerSecond < MinShotsPerSecond){
				shotsPerSecond = MinShotsPerSecond;
			}

			//A second floor on the far end: above 1000 shots per second the interval
			//truncates to 0, and the cooldown test below it is an unsigned comparison
			//that is never true against 0 - so a high enough dexterity would fire a
			//projectile every single frame with no cap at all.
			int shotInterval = static_cast<int>(1000.0f / shotsPerSecond);
			if (shotInterval < MinShotIntervalMs){
				shotInterval = MinShotIntervalMs;
			}

			return shotInterval;
		}

		//If the shooter has a sprite, the shot starts in the middle of it rather than
		//at the transform's top-left corner
		glm::vec2 GetMuzzlePosition(Entity entity, const TransformComponent& transform){
			glm::vec2 projectilePosition = transform.position;
			if (entity.HasComponent<SpriteComponent>()){
				const auto sprite = entity.GetComponent<SpriteComponent>();
				projectilePosition.x += (transform.scale.x * sprite.width / 2);
				projectilePosition.y += (transform.scale.y * sprite.height / 2);
			}
			return projectilePosition;
		}

		void EmitProjectile(Entity entity, glm::vec2 position, glm::vec2 velocity, const ProjectileProfile& profile){

			const int finalDamage = RollScaledDamage(entity, profile.minDamage, profile.maxDamage, rng);
			SpawnProjectile(registry, assetStore, entity, position, velocity, finalDamage, profile.lifetimeMs, profile.texture);
			
			eventBus->EmitEvent<ProjectileFiredEvent>(position, profile.shotSound);
		}

		Registry& registry;
		const SDL_Rect& camera;
		const TileMap& tileMap;            
		const ItemDatabase& itemDatabase;  
		AssetStore& assetStore;
		std::unique_ptr<EventBus>& eventBus;

		static constexpr float BaseShotsPerSecond = 1.0f;        //fire rate with no dexterity at all
		static constexpr float DexterityPerStep = 75.0f;         //dexterity needed for one full step
		static constexpr float MinShotsPerSecond = 0.1f;         //floor, so the interval can never blow up
		static constexpr int MinShotIntervalMs = 1;              //floor, so the interval can never reach zero
		std::mt19937 rng{ std::random_device{}() };


		//The bullet a shooter fires: its equipped weapon's, otherwise its own emitter's
		//(the unarmed player, and every enemy)
		ProjectileProfile GetProfile(Entity entity, const ProjectileEmitterComponent& emitter) {
			ProjectileProfile profile;
			profile.texture = emitter.projectileTexture;
			profile.shotSound = emitter.shotSound;
			profile.minDamage = emitter.projectileDamage;
			profile.maxDamage = emitter.projectileDamage;
			profile.speed = glm::length(emitter.projectileVelocity);
			profile.lifetimeMs = emitter.projectileDuration;
			profile.shotsPerSecond = BaseShotsPerSecond;

			if (!entity.HasComponent<EquipmentComponent>()) {
				return profile;
			}

			const auto& equipment = entity.GetComponent<EquipmentComponent>();
			for (const auto& slot : equipment.slots) {
				if (slot.accepts != "weapon" || slot.itemId.empty()) {
					continue;
				}

				const ItemDefinition* definition = itemDatabase.Find(slot.itemId);
				if (definition == nullptr || !definition->isWeapon) {
					continue;
				}

				const WeaponStats& weapon = definition->weapon;
				profile.texture = weapon.projectileTexture;
				profile.shotSound = weapon.shotSound;
				profile.minDamage = weapon.minDamage;
				profile.maxDamage = weapon.maxDamage;
				profile.speed = weapon.projectileSpeed;
				profile.shotsPerSecond = weapon.shotPerSecond;
				profile.lifetimeMs = static_cast<int>(std::lround(weapon.rangeTiles * tileMap.TileWorldSize() / weapon.projectileSpeed * 1000.0));
				break;
			}
			return profile;
		}

};
#endif

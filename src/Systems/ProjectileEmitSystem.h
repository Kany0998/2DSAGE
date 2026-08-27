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
#include <SDL.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>

// Two kinds of emitter live here:
//  - the player (CameraHollderComponent): holds the left mouse button to fire
//    continuously toward the cursor, at a rate driven by dexterityPower.
//  - everything else (enemies): fires on the fixed repeatRate timer loaded from Lua,
//    straight along its configured projectileVelocity. Enemies have no dexterity,
//    so their rate is deliberately left alone.
class ProjectileEmitSystem
{
	public:
		// Registry + camera are kept as references for the lifetime of the system:
		// Update() polls the mouse itself (that's what makes hold-to-fire possible at
		// all - SDL_MOUSEBUTTONDOWN only fires once per physical press), and the
		// cursor's screen position needs the camera offset to become a world-space aim
		// target, the same way SpecialAbilitySystem does it.
		ProjectileEmitSystem(Registry& registry, const SDL_Rect& camera)
			: registry(registry), camera(camera) {}

		void Update()
		{
			// Snapshot first: emitting a projectile adds a TransformComponent to a
			// brand-new entity, which can grow/reallocate that component's pool.
			// Doing that while still iterating a live view over TransformComponent is
			// unsafe (can invalidate the iteration mid-loop) - materializing the
			// candidates first, like CollisionSystem does, avoids that entirely.
			std::vector<Entity> emitters;
			for (auto rawEntity : registry.Raw().view<ProjectileEmitterComponent, TransformComponent>())
			{
				emitters.emplace_back(rawEntity, &registry);
			}

			for (auto entity : emitters)
			{
				auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
				//deliberately a copy, not a reference: the pool it lives in can
				//reallocate while the projectile below is being created
				const auto transform = entity.GetComponent<TransformComponent>();

				if (entity.HasComponent<CameraHollderComponent>())
				{
					UpdateMouseAimedEmitter(entity, projectileEmitter, transform);
				}
				else
				{
					UpdateAutomaticEmitter(entity, projectileEmitter, transform);
				}
			}
		}

	private:
		//Player shooting: fires for as long as the left button is held down, aimed at
		//wherever the cursor currently is. repeatRate is ignored here on purpose - the
		//player's fire rate comes from dexterity (see GetShotIntervalMs) instead.
		void UpdateMouseAimedEmitter(Entity entity, ProjectileEmitterComponent& projectileEmitter, const TransformComponent& transform)
		{
			//don't shoot through an ImGui window - the same guard Game::ProccessInput() applies
			//to the click event. ImGui runs a frame unconditionally now, so WantCaptureMouse is
			//always current and needs no debug-mode qualifier.
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return;
			}

			int mouseX, mouseY;
			const Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
			if (!(mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)))
			{
				return;
			}

			if (SDL_GetTicks() - projectileEmitter.lastEmittedTime < GetShotIntervalMs(entity))
			{
				return;
			}

			const glm::vec2 projectilePosition = GetMuzzlePosition(entity, transform);

			//mouse position is screen-space - camera offset turns it into the world-space
			//point the shot should travel toward
			const glm::vec2 aimTarget(mouseX + camera.x, mouseY + camera.y);
			glm::vec2 aimDirection = aimTarget - projectilePosition;
			const float distance = glm::length(aimDirection);
			if (distance < 0.0001f)
			{
				return; //cursor sits right on top of the shooter - no direction to aim in
			}
			aimDirection /= distance;

			//projectileVelocity is used as a speed magnitude here rather than a per-axis velocity
			const float speed = glm::length(projectileEmitter.projectileVelocity);

			EmitProjectile(entity, projectileEmitter, transform, projectilePosition, aimDirection * speed);
			projectileEmitter.lastEmittedTime = SDL_GetTicks();
		}

		//Enemy shooting: unchanged fixed-interval timer along a fixed velocity
		void UpdateAutomaticEmitter(Entity entity, ProjectileEmitterComponent& projectileEmitter, const TransformComponent& transform)
		{
			if (projectileEmitter.repeatRate == 0)
			{
				return;
			}

			//check if it is time to re-emit a new projectile
			if (SDL_GetTicks() - projectileEmitter.lastEmittedTime <= projectileEmitter.repeatRate)
			{
				return;
			}

			const glm::vec2 projectilePosition = GetMuzzlePosition(entity, transform);
			EmitProjectile(entity, projectileEmitter, transform, projectilePosition, projectileEmitter.projectileVelocity);
			projectileEmitter.lastEmittedTime = SDL_GetTicks();
		}

		//Milliseconds the shooter has to wait between shots, derived from dexterity:
		//BaseShotsPerSecond at 0 dexterity, plus DexterityShotsPerSecond extra shots per
		//second for every DexterityPerStep points. All of it stays in float - integer
		//division here would collapse whole ranges of dexterity into an identical rate.
		int GetShotIntervalMs(Entity entity)
		{
			float shotsPerSecond = BaseShotsPerSecond;
			if (entity.HasComponent<AttributesComponent>())
			{
				const auto& attributes = entity.GetComponent<AttributesComponent>();
				shotsPerSecond += (attributes.dexterityPower / DexterityPerStep) * DexterityShotsPerSecond;
			}

			//guards against a zero/negative rate if dexterity ever goes negative
			if (shotsPerSecond < MinShotsPerSecond)
			{
				shotsPerSecond = MinShotsPerSecond;
			}

			//A second floor on the far end: above 1000 shots per second the interval
			//truncates to 0, and the cooldown test below it is an unsigned comparison
			//that is never true against 0 - so a high enough dexterity would fire a
			//projectile every single frame with no cap at all.
			int shotInterval = static_cast<int>(1000.0f / shotsPerSecond);
			if (shotInterval < MinShotIntervalMs)
			{
				shotInterval = MinShotIntervalMs;
			}

			return shotInterval;
		}

		//If the shooter has a sprite, the shot starts in the middle of it rather than
		//at the transform's top-left corner
		glm::vec2 GetMuzzlePosition(Entity entity, const TransformComponent& transform)
		{
			glm::vec2 projectilePosition = transform.position;
			if (entity.HasComponent<SpriteComponent>())
			{
				const auto sprite = entity.GetComponent<SpriteComponent>();
				projectilePosition.x += (transform.scale.x * sprite.width / 2);
				projectilePosition.y += (transform.scale.y * sprite.height / 2);
			}
			return projectilePosition;
		}

		void EmitProjectile(Entity entity, const ProjectileEmitterComponent& projectileEmitter, const TransformComponent& transform, glm::vec2 position, glm::vec2 velocity)
		{
			//Damage is scaled by the shooter's attackPower attribute, if it has one
			int scaledDamage = static_cast<int>(projectileEmitter.projectileDamage * 0.5f);
			if (entity.HasComponent<AttributesComponent>())
			{
				const auto& attributes = entity.GetComponent<AttributesComponent>();
				scaledDamage = static_cast<int>(scaledDamage + (scaledDamage * (attributes.attackPower / 25.0f)));
			}

			//point the sprite in the direction it's travelling
			const double rotation = glm::degrees(std::atan2(velocity.y, velocity.x));

			Entity projectile = registry.CreateEntity();
			projectile.Group("projectiles");
			projectile.AddComponent<TransformComponent>(position, glm::vec2(transform.scale.x, transform.scale.y), rotation);
			projectile.AddComponent<RigidBodyComponent>(velocity);
			projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 8);
			projectile.AddComponent<BoxColliderComponent>(4, 4);
			//`entity` is recorded as the owner so whatever this shot kills can be
			//credited back to the shooter
			projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, scaledDamage, projectileEmitter.projectileDuration, entity);
		}

		Registry& registry;
		const SDL_Rect& camera;

		static constexpr float BaseShotsPerSecond = 1.0f;        //fire rate with no dexterity at all
		static constexpr float DexterityPerStep = 75.0f;         //dexterity needed for one full step
		static constexpr float DexterityShotsPerSecond = 5.0f;   //extra shots per second per full step
		static constexpr float MinShotsPerSecond = 0.1f;         //floor, so the interval can never blow up
		static constexpr int MinShotIntervalMs = 1;              //floor, so the interval can never reach zero
};
#endif

#ifndef SPECIALABILITYSYSTEM_H
#define SPECIALABILITYSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/ManaComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/CameraHollderComponent.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <cmath>

// Mana-gated special ability: SPACE casts a burst of shots outward from the
// mouse cursor's world position, in a full circle, that expire after a
// short lifetime - same expiry mechanism ProjectileLifeCycleSystem already
// uses for regular bullets (ProjectileComponent::duration).
class SpecialAbilitySystem
{
	public:
		// Needs registry + camera kept alive for OnKeyPressed, since that's an
		// event-callback with a fixed (KeyPressedEvent&) signature and can't
		// receive them as call parameters like Update() does elsewhere.
		SpecialAbilitySystem(Registry& registry, const SDL_Rect& camera) : registry(registry), camera(camera) {}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
		{
			eventBus->SubscribeToEvent<KeyPressedEvent>(this, &SpecialAbilitySystem::OnKeyPressed);
		}

		void OnKeyPressed(KeyPressedEvent& event)
		{
			if (event.symbol != SDLK_SPACE)
			{
				return;
			}

			// Snapshot first: the burst below creates new entities with a TransformComponent,
			// which can grow/reallocate that component's pool. Doing that while still
			// iterating a live view over TransformComponent is unsafe - especially here,
			// where up to ShotCount entities get created per caster in one pass.
			std::vector<Entity> casters;
			for (auto rawEntity : registry.Raw().view<ManaComponent, TransformComponent, CameraHollderComponent, AttributesComponent>())
			{
				casters.emplace_back(rawEntity, &registry);
			}

			for (auto entity : casters)
			{
				auto& mana = entity.GetComponent<ManaComponent>();

				int manaCastCost = ManaCost;
				if (mana.manaPoints < manaCastCost)
				{
					continue; //not enough mana to cast
				}
				mana.manaPoints -= manaCastCost;

				//mouse position is screen-space - convert to world-space the same way
				//Game::ProccessInput() does for MouseButtonPressedEvent
				int mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);
				glm::vec2 explosionCenter(mouseX + camera.x, mouseY + camera.y);

				//reuse the player's own shot damage/friendliness rather than adding
				//a separate ability-damage field for now
				int damage = 1;
				bool isFriendly = true;
				if (entity.HasComponent<ProjectileEmitterComponent>())
				{
					const auto& emitter = entity.GetComponent<ProjectileEmitterComponent>();
					damage = emitter.projectileDamage;
					isFriendly = emitter.isFriendly;
				}

				for (int i = 0; i < ShotCount; i++)
				{
					double angle = (2.0 * Pi * i) / ShotCount;
					glm::vec2 direction(std::cos(angle), std::sin(angle));
					glm::vec2 velocity = direction * ShotSpeed;
					double rotation = glm::degrees(angle); //point the sprite in its travel direction

					Entity shot = registry.CreateEntity();
					shot.Group("projectiles");
					shot.AddComponent<TransformComponent>(explosionCenter, glm::vec2(5.0, 5.0), rotation);
					shot.AddComponent<RigidBodyComponent>(velocity);
					shot.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 8);
					shot.AddComponent<BoxColliderComponent>(4, 4);
					shot.AddComponent<ProjectileComponent>(isFriendly, damage, ShotDuration);
				}
			}
		}

	private:
		Registry& registry;
		const SDL_Rect& camera;

		static constexpr int ShotCount = 20;
		static constexpr float ShotSpeed = 30.0f;   //pixels per second
		static constexpr int ShotDuration = 10000;    //milliseconds (matches the "for 1 second" ask)
		static constexpr int ManaCost = 200;         //out of maxManaPoints
		static constexpr double Pi = 3.14159265358979323846;
};

#endif

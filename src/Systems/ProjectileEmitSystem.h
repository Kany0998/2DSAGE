#ifndef PROJECTILEEMITSYSTEM_H
#define PROJECTILEEMITSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/MouseButtonPressedEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/CameraHollderComponent.h"
#include "../Components/AttributesComponent.h"
#include <SDL.h>
#include <cmath>

class ProjectileEmitSystem
{
	public:
		// Needs a registry reference kept alive for OnMouseButtonPressed, since
		// that's an event-callback with a fixed (MouseButtonPressedEvent&)
		// signature and can't receive the registry as a call parameter like
		// Update() does.
		explicit ProjectileEmitSystem(Registry& registry) : registry(registry) {}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
		{
            eventBus->SubscribeToEvent<MouseButtonPressedEvent>(this, &ProjectileEmitSystem::OnMouseButtonPressed);
        }


        void OnMouseButtonPressed(MouseButtonPressedEvent& event) {
            if (event.button == SDL_BUTTON_LEFT)
            {
                // Snapshot first: creating the projectile below adds a TransformComponent
                // to a brand-new entity, which can grow/reallocate that component's pool.
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
                    if (entity.HasComponent<CameraHollderComponent>())
                    {
                        const auto projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
                        const auto transform = entity.GetComponent<TransformComponent>();

                        // If parent entity has sprite, start the projectile position in the middle of the entity
                        glm::vec2 projectilePosition = transform.position;
                        if (entity.HasComponent<SpriteComponent>())
                        {
                            auto sprite = entity.GetComponent<SpriteComponent>();
                            projectilePosition.x += (transform.scale.x * sprite.width / 2);
                            projectilePosition.y += (transform.scale.y * sprite.height / 2);
                        }

                        // Aim toward wherever the mouse was clicked (already world-space),
                        // using the configured projectileVelocity as a speed magnitude
                        // rather than a per-axis velocity.
                        glm::vec2 aimDirection = event.worldPosition - projectilePosition;
                        float distance = glm::length(aimDirection);
                        if (distance < 0.0001f)
                        {
                            continue; //clicked right on top of the shooter - no direction to aim in
                        }
                        aimDirection /= distance;

                        float speed = glm::length(projectileEmitter.projectileVelocity);
                        glm::vec2 projectileVelocity = aimDirection * speed;

                        //point the sprite in the direction it's travelling
                        double rotation = glm::degrees(std::atan2(projectileVelocity.y, projectileVelocity.x));

						//Damage is scaled by the player's attackPower attribute, if it has one
                        int scaledDamage = static_cast<int>(projectileEmitter.projectileDamage * 0.5f);
                        if (entity.HasComponent<AttributesComponent>()) {
                            const auto& attributes = entity.GetComponent<AttributesComponent>();
                            scaledDamage = static_cast<int>(scaledDamage + (scaledDamage * (attributes.attackPower / 25.0f)));
                        }

                        // Create new projectile entity and add it to the world
                        Entity projectile = entity.registry->CreateEntity();
                        projectile.Group("projectiles");
                        projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(transform.scale.x, transform.scale.y), rotation);
                        projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
                        projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 8);
                        projectile.AddComponent<BoxColliderComponent>(4, 4);
                        projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, scaledDamage, projectileEmitter.projectileDuration);
                    }
                }
            }
        }

		void Update()
		{
			// Same reallocation-during-iteration hazard as OnMouseButtonPressed -
			// snapshot the emitters before creating any new projectiles.
			std::vector<Entity> emitters;
			for (auto rawEntity : registry.Raw().view<ProjectileEmitterComponent, TransformComponent>())
			{
				emitters.emplace_back(rawEntity, &registry);
			}

			for (auto entity : emitters)
			{
				auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
				const auto transform = entity.GetComponent<TransformComponent>();

                if (projectileEmitter.repeatRate == 0)
                {
                    continue;
                }

				//check if it time to re-emit a new projectile
				if (SDL_GetTicks() - projectileEmitter.lastEmittedTime > projectileEmitter.repeatRate)
				{
					glm::vec2 projectilePosition = transform.position;
					if (entity.HasComponent<SpriteComponent>())
					{
						const auto sprite = entity.GetComponent<SpriteComponent>();
						projectilePosition.x += (transform.scale.x * sprite.width / 2);
						projectilePosition.y += (transform.scale.y * sprite.height / 2);
					}

					//point the sprite in the direction it's travelling
					double rotation = glm::degrees(std::atan2(projectileEmitter.projectileVelocity.y, projectileEmitter.projectileVelocity.x));

                    //Damage is scaled by the entity's attackPower attribute, if it has one
                    int scaledDamage = static_cast<int>(projectileEmitter.projectileDamage * 0.5f);
                    if (entity.HasComponent<AttributesComponent>()) {
                        const auto& attributes = entity.GetComponent<AttributesComponent>();
                        scaledDamage = static_cast<int>(scaledDamage + (scaledDamage * (attributes.attackPower / 25.0f)));
                    }

					Entity projectile = registry.CreateEntity();
					projectile.Group("projectiles");
					projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(transform.scale.x, transform.scale.y), rotation);
					projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
					projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 8);
					projectile.AddComponent<BoxColliderComponent>(4, 4);
					projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, scaledDamage, projectileEmitter.projectileDuration);

					//update projectile emmiter component last emitted time
					projectileEmitter.lastEmittedTime = SDL_GetTicks();
				}
			}
		}

	private:
		Registry& registry;
};
#endif

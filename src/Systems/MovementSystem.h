#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"


class MovementSystem
{
	public:
		MovementSystem() = default;

		void SubscribeToEvents(const std::unique_ptr<EventBus>& eventBus)
		{
			eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollision);
		}

		void onCollision(CollisionEvent& ev)
		{
			Entity a = ev.a;
			Entity b = ev.b;

			if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles"))
			{
				OnEnemyHitsObstacle(a, b);
			}

			if (b.BelongsToGroup("enemies") && a.BelongsToGroup("obstacles"))
			{
				OnEnemyHitsObstacle(b, a);
			}
		}

		void Update(Registry& registry, double deltaTime)
		{
			//Loop all entites that have Transform + RigidBody + Sprite
			for (auto rawEntity : registry.Raw().view<TransformComponent, RigidBodyComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				auto& transform = entity.GetComponent<TransformComponent>();
				const auto rigidbody = entity.GetComponent<RigidBodyComponent>();
				auto sprite = entity.GetComponent<SpriteComponent>();

				transform.position.x += rigidbody.velocity.x * deltaTime;
				transform.position.y += rigidbody.velocity.y * deltaTime;

				//
				int paddingLeft = 0;
				int paddingTop = 0;
				int paddingRight = Game::mapWidth - sprite.width * transform.scale.x;
				int paddingBottom = Game::mapHeight - sprite.height * transform.scale.y;
				if (entity.HasTag("player"))
				{
					transform.position.x = transform.position.x <= paddingLeft ? paddingLeft : transform.position.x;
					transform.position.x = transform.position.x >= paddingRight ? paddingRight : transform.position.x;
					transform.position.y = transform.position.y <= paddingTop ? paddingTop : transform.position.y;
					transform.position.y = transform.position.y >= paddingBottom ? paddingBottom : transform.position.y;
				}


				//Kills enmys outside the map
				bool isEntityOutsideMap = (transform.position.x < -150||
					transform.position.x > Game::mapWidth +150||
					transform.position.y < -150||
					transform.position.y > Game::mapHeight + 150
					);

				if (isEntityOutsideMap && !entity.HasTag("player"))
				{
					entity.Kill();
				}
			}
		}

		void OnEnemyHitsObstacle(Entity enemy, Entity obstacle)
		{
			if(enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>())
			{
				auto& rigidbody = enemy.GetComponent<RigidBodyComponent>();
				auto& sprite = enemy.GetComponent<SpriteComponent>();
				if(rigidbody.velocity.x != 0)
				{
					rigidbody.velocity.x *= -1;
					sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
				}

				if(rigidbody.velocity.y != 0)
				{
					rigidbody.velocity.y *= -1;
					sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
				}
			}
		}
 };

#endif


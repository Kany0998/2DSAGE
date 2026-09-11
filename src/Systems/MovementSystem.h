#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../TileMap/TileMap.h"


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

		void Update(Registry& registry, double deltaTime, const TileMap& tileMap)
		{
			//Loop all entites that have Transform + RigidBody + Sprite
			for (auto rawEntity : registry.Raw().view<TransformComponent, RigidBodyComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				auto& transform = entity.GetComponent<TransformComponent>();
				const auto rigidbody = entity.GetComponent<RigidBodyComponent>();
				auto sprite = entity.GetComponent<SpriteComponent>();

				//Movement speed is scaled by the entity's speedPower attribute, if it has
				//one. Entities without attributes (projectiles, obstacles) keep a plain
				//multiplier of 1 and move at exactly their rigidbody velocity.
				float speedMultiplier = BaseSpeedMultiplier;
				if (entity.HasComponent<AttributesComponent>()) {
					const auto& attribute = entity.GetComponent<AttributesComponent>();

					//Kept in float on purpose: written as an int division, every speedPower
					//from 1 to 49 would truncate to 0 and the attribute would do nothing at
					//all, while 50 would jump straight to double speed.
					speedMultiplier += attribute.speedPower / SpeedPowerPerStep;

					//Floor rather than clamping the attribute itself - a movement system has
					//no business rewriting an entity's stats, and without this a speedPower
					//below -50 would make the entity travel backwards.
					if (speedMultiplier < MinSpeedMultiplier) {
						speedMultiplier = MinSpeedMultiplier;
					}
				}

				bool hasMovementType = entity.HasComponent<MovementTypeComponent>();
				int movmentType = hasMovementType ? entity.GetComponent<MovementTypeComponent>().movementType : MovementType_Ground;

				double candidateX = transform.position.x + rigidbody.velocity.x * speedMultiplier * deltaTime;
				double candidateY = transform.position.y + rigidbody.velocity.y * speedMultiplier * deltaTime;

				//ignores terrain blocking if the entity has no movement type, e.g. projectiles and obstacles
				if (!hasMovementType) {
					transform.position.x = candidateX;
					transform.position.y = candidateY;
				}

				else {
					int halfWidth = sprite.width * transform.scale.x / 2;
					int halfHeight = sprite.height * transform.scale.y / 2;

					//X axis : candiadte testes against current y
					if (!tileMap.isBlockedAtWorld(candidateX + halfWidth, transform.position.y + halfHeight, movmentType)) {
						transform.position.x = candidateX;
					}
					//Y axis : candidate tests against current x
					if (!tileMap.isBlockedAtWorld(transform.position.x + halfWidth, candidateY + halfHeight, movmentType)) {
						transform.position.y = candidateY;
					}
					
				}
				
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

	private:
		static constexpr float BaseSpeedMultiplier = 1.0f;	//movement speed with no speedPower at all
		static constexpr float SpeedPowerPerStep = 50.0f;	//speedPower needed to add one full extra unit of speed
		static constexpr float MinSpeedMultiplier = 0.1f;	//floor, so a negative speedPower can't reverse movement
 };

#endif


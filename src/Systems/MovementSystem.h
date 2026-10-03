#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/PushableComponent.h"
#include "../TileMap/TileMap.h"
#include "../Physics/PushResolver.h"
#include "../Utils/EntityGeometry.h"
#include <cmath>


class MovementSystem
{
	public:
		MovementSystem() = default;


		void Update(Registry& registry, double deltaTime, const TileMap& tileMap)
		{
			//Loop all entites that have Transform + RigidBody + Sprite
			for (auto rawEntity : registry.Raw().view<TransformComponent, RigidBodyComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
				const auto& sprite = entity.GetComponent<SpriteComponent>();

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
				int movementType = hasMovementType ? entity.GetComponent<MovementTypeComponent>().movementType : MovementType_Ground;

				double speedTileMultiplier = 1.0;
				double friction = 1.0;
				double acceleration = 0.0;
				
				if (hasMovementType && movementType == MovementType_Ground) {
					glm::vec2 entityCenter = EntityCenter(transform, sprite);
					int const col = tileMap.colAt(entityCenter.x);
					int const row = tileMap.rowAt(entityCenter.y);
					speedTileMultiplier = tileMap.SpeedMultiplierAt(col, row);
					friction = tileMap.FrictionAt(col, row);
					acceleration = tileMap.AccelerationAt(col, row);
				}

				speedMultiplier = speedMultiplier * speedTileMultiplier;
				glm::vec2 desiredSpeed = rigidbody.velocity * speedMultiplier;


				//acceleration of 0 means "not slippery": the entity simply gets what it asks for,
				//which is how every unauthored tile behaves
				if (acceleration <= 0) {
					rigidbody.actualVelocity = desiredSpeed;
				}

				else {
					//Slippery ground: the entity does not get the velocity it asks for, it is
					//pushed towards it and keeps whatever it had
					const float desiredLength = glm::length(desiredSpeed);
					const float previousLength = glm::length(rigidbody.actualVelocity);

					if (desiredLength > 0.0001f) {
						const glm::vec2 direction = desiredSpeed / desiredLength;

						//Push only until it is already moving that fast in the direction it is
						//being pushed. Slowing down is friction's job - capping the velocity
						//itself would erase momentum the moment the AI asks for less, and
						//enemies would hardly slide at all
						const float speedAlongDirection = glm::dot(rigidbody.actualVelocity, direction);

						if (speedAlongDirection < desiredLength) {
							rigidbody.actualVelocity += direction * static_cast<float>(acceleration * deltaTime);
						}
					}

					//Turning must not create speed: pushing north-east while already moving east
					//adds to both axes and the total would exceed what was asked for. The cap is
					//whichever is larger, the speed asked for or the speed already carried, so
					//momentum survives while boosts do not
					const float allowedLength = std::max(desiredLength, previousLength);
					const float newLength = glm::length(rigidbody.actualVelocity);

					if (newLength > allowedLength && newLength > 0.0001f) {
						rigidbody.actualVelocity = rigidbody.actualVelocity / newLength * allowedLength;
					}

					//Friction is per second, not per frame: pow keeps the same ice behaving the
					//same way at 60 and at 144 frames per second
					rigidbody.actualVelocity *= static_cast<float>(std::pow(friction, deltaTime));

					//Exponential decay never reaches zero, so below a pixel per second it is
					//simply not moving
					if (glm::length(rigidbody.actualVelocity) < 1.0f) {
						rigidbody.actualVelocity = glm::vec2(0.0f, 0.0f);
					}
				}
				

				double candidateX = transform.position.x + rigidbody.actualVelocity.x * deltaTime;
				double candidateY = transform.position.y + rigidbody.actualVelocity.y * deltaTime;

				//ignores terrain blocking if the entity has no movement type
				if (!hasMovementType) {
					transform.position.x = candidateX;
					transform.position.y = candidateY;
				}

				//A projectile never pushes and is never pushed: it flies on, or dies on terrain that stops bullets
				else if (entity.HasComponent<ProjectileComponent>()) {
					const double halfWidth = sprite.width * transform.scale.x / 2.0;
					const double halfHeight = sprite.height * transform.scale.y / 2.0;

					//Tested where it is going, in one step - a bullet does not slide along walls
					if (tileMap.isBlockedAtWorld(candidateX + halfWidth, candidateY + halfHeight, movementType)) {
						entity.Kill();
					}
					else {
						transform.position.x = static_cast<float>(candidateX);
						transform.position.y = static_cast<float>(candidateY);
					}
				}

				else {

					int halfWidth = sprite.width * transform.scale.x / 2;
					int halfHeight = sprite.height * transform.scale.y / 2;

					const bool isPushable = entity.HasComponent<PushableComponent>();

					//X axis : candidate tests against current y
					if (!tileMap.isBlockedAtWorld(candidateX + halfWidth, transform.position.y + halfHeight, movementType)) {
						
						//A sliding crate stops at a player or enemy, on this axis only
						if (isPushable && candidateX != transform.position.x 
							&& PushResolver::IsBlockedByBody(registry, entity, candidateX, transform.position.y, true)
							) {
							rigidbody.actualVelocity.x = 0.0f;

						}
						//Returns the distance allowed, so it is added, not assigned
						else {
							transform.position.x += static_cast<float>(PushResolver::PushAlongAxis(registry, tileMap, entity, candidateX, transform.position.y, candidateX - transform.position.x, true, deltaTime));
						}
					}
					//Y axis : candidate tests against current x
					if (!tileMap.isBlockedAtWorld(transform.position.x + halfWidth, candidateY + halfHeight, movementType)) {

						//A sliding crate stops at a player or enemy, on this axis only
						if (isPushable && candidateY != transform.position.y
							&& PushResolver::IsBlockedByBody(registry, entity, transform.position.x, candidateY, false)
							) {
							rigidbody.actualVelocity.y = 0.0f;

						}
						
						else {
							transform.position.y += static_cast<float>(PushResolver::PushAlongAxis(registry, tileMap, entity, transform.position.x, candidateY, candidateY - transform.position.y, false, deltaTime));
						}
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

	private:
		static constexpr float BaseSpeedMultiplier = 1.0f;	//movement speed with no speedPower at all
		static constexpr float SpeedPowerPerStep = 50.0f;	//speedPower needed to add one full extra unit of speed
		static constexpr float MinSpeedMultiplier = 0.1f;	//floor, so a negative speedPower can't reverse movement
 };

#endif


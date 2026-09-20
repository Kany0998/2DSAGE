#ifndef AISYSTEM_H
#define AISYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/AIComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Pathfinding/Pathfinder.h"
#include "../TileMap/TileMap.h"
#include "../TileMap/MovementType.h"
#include <string>
#include <algorithm>
#include <cmath>

class AISystem
{
	public:
		AISystem() = default;

		void Update(Registry& registry, double deltaTime, const TileMap& tileMap, Pathfinder& pathfinder) {
			
			//No player, no chasing: stop everthing that is ai driven
			if (!registry.HasEntityWithTag("player")) {
				for (auto rawEntity : registry.Raw().view<AIComponent, RigidBodyComponent>()) {
					Entity entity(rawEntity, &registry);
					entity.GetComponent<RigidBodyComponent>().velocity = glm::vec2(0.0, 0.0);
					entity.GetComponent<AIComponent>().isChasing = false;
				}
				return;
			}

			Entity player = registry.GetEntityByTag("player");
			if (!player.IsAlive()) {
				return;
			}

			//Center of sprite: the point terrarin collison and pathfinder use
			const auto& playerTransform = player.GetComponent<TransformComponent>();
			const auto& playerSprite = player.GetComponent<SpriteComponent>();

			const glm::vec2 playerCenter = EntityCenter(playerTransform, playerSprite);

			int playerCol = tileMap.colAt(playerCenter.x);
			int playerRow = tileMap.rowAt(playerCenter.y);
			int playerCellIndex = tileMap.Index(playerCol, playerRow);

			int searchesThisFrame = 0;

			for (auto rawEntity : registry.Raw().view<AIComponent, TransformComponent, SpriteComponent, RigidBodyComponent, MovementTypeComponent>()) {
				Entity entity(rawEntity, &registry);
				auto& enemyAI = entity.GetComponent<AIComponent>();
				const auto& enemyTransform = entity.GetComponent<TransformComponent>();
				const auto& enemySprite = entity.GetComponent<SpriteComponent>();
				auto& enemyRigidBody = entity.GetComponent<RigidBodyComponent>();
				const auto& enemyMovementType = entity.GetComponent<MovementTypeComponent>();

				if (enemyAI.scriptControlsVelocity) { continue; }
				
				const glm::vec2 enemyCenter = EntityCenter(enemyTransform, enemySprite);
				int enemyCol = tileMap.colAt(enemyCenter.x);
				int enemyRow = tileMap.rowAt(enemyCenter.y);


				double dx = playerCenter.x - enemyCenter.x;
				double dy = playerCenter.y - enemyCenter.y;

				double distanceSquared = dx * dx + dy * dy;

				double range = enemyAI.isChasing ? enemyAI.detectionRange * 1.2 : enemyAI.detectionRange;

				const bool wasChasing = enemyAI.isChasing;
				enemyAI.isChasing = distanceSquared < range * range;

				if (wasChasing != enemyAI.isChasing) {
					Logger::Log("Enemy " + std::to_string(entity.GetId()) + (enemyAI.isChasing ? "Stared Chasing" : "Stopped Chasing"));
				}

				if (!enemyAI.isChasing) {
					enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
					enemyAI.path.clear();
					enemyAI.waypointIndex = 0;
					enemyAI.lastTargetCell = -1;
				}


				else {
					enemyAI.repathTimer += deltaTime;
					bool shouldSearch = playerCellIndex != enemyAI.lastTargetCell || enemyAI.repathTimer >= 0.5;

					if (shouldSearch) {
						if (searchesThisFrame >= 2) {
							continue;
						}

						pathfinder.FindPath(tileMap, enemyCol, enemyRow, playerCol, playerRow, enemyMovementType.movementType, enemyAI.path);

						searchesThisFrame++;
						enemyAI.repathTimer = 0.0;
						enemyAI.lastTargetCell = playerCellIndex;
						enemyAI.waypointIndex = 0;
						Logger::Log("EntityId: " + std::to_string(entity.GetId()) + " Path size: " + std::to_string(enemyAI.path.size()));
					}

					if (enemyAI.waypointIndex >= static_cast<int>(enemyAI.path.size())) {
						enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
						continue;
					}

					const bool isLastWaypoint = enemyAI.waypointIndex == static_cast<int>(enemyAI.path.size()) - 1;
					double targetX = 0;
					double targetY = 0;
					double tileWorldSize = tileMap.TileWorldSize();
					if (isLastWaypoint) {
						targetX = playerCenter.x;
						targetY = playerCenter.y;
					}

					else {
						int cellIndex = enemyAI.path[enemyAI.waypointIndex];
						targetX = tileMap.IndexToCol(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
						targetY = tileMap.IndexToRow(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
					}

					double wdx = targetX - enemyCenter.x;
					double wdy = targetY - enemyCenter.y;
					double waypointDistanceSquared = wdx * wdx + wdy * wdy;

					if (!isLastWaypoint) {
						double arrivalRadius = std::max(tileWorldSize * 0.25, enemyAI.movementSpeed * deltaTime * 1.5);

						if (arrivalRadius * arrivalRadius > waypointDistanceSquared) {
							enemyAI.waypointIndex++;
						}
					}

					const double enemyHalfSize = enemySprite.width * enemyTransform.scale.x / 2.0;
					const double playerHalfSize = playerSprite.width * playerTransform.scale.x / 2.0;
					const double contactDistance = playerHalfSize + enemyHalfSize;
					const double effectiveStopRange = std::max(contactDistance, enemyAI.stopRange);

					double length = std::sqrt(waypointDistanceSquared);

					if (distanceSquared < effectiveStopRange * effectiveStopRange) {
						enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
						continue;
					}

					if (length < 0.001) {
						enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
						continue;
					}

					double speedThisFrame = enemyAI.movementSpeed;
					if (speedThisFrame * deltaTime > length) {
						speedThisFrame = length / deltaTime;
					}

					double directionX = wdx / length;
					double directionY = wdy / length;
					enemyRigidBody.velocity = glm::vec2(directionX * speedThisFrame, directionY * speedThisFrame);

				}
			}
		}


	private:
		static glm::vec2 EntityCenter(const TransformComponent& transform, const SpriteComponent& sprite) {
			return glm::vec2(
				transform.position.x + sprite.width * transform.scale.x / 2.0,
				transform.position.y + sprite.height * transform.scale.y / 2.0
			);
		}


};

#endif // !AISYSTEM_H

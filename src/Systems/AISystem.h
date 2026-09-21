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

		//Flip to see every search in the console; used for the repath measurements
		bool logSearches = false;

		void Update(Registry& registry, double deltaTime, const TileMap& tileMap, Pathfinder& pathfinder) {

			//No player, no chasing: stop everything that is ai driven
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

			//Center of sprite: the point terrain collision and the pathfinder both use
			const auto& playerTransform = player.GetComponent<TransformComponent>();
			const auto& playerSprite = player.GetComponent<SpriteComponent>();

			const glm::vec2 playerCenter = EntityCenter(playerTransform, playerSprite);

			//The player is looked up once per frame, not once per enemy
			const int playerCol = tileMap.colAt(playerCenter.x);
			const int playerRow = tileMap.rowAt(playerCenter.y);
			const int playerCellIndex = tileMap.Index(playerCol, playerRow);

			const double tileWorldSize = tileMap.TileWorldSize();

			int searchesThisFrame = 0;

			for (auto rawEntity : registry.Raw().view<AIComponent, TransformComponent, SpriteComponent, RigidBodyComponent, MovementTypeComponent>()) {
				Entity entity(rawEntity, &registry);
				auto& enemyAI = entity.GetComponent<AIComponent>();
				const auto& enemyTransform = entity.GetComponent<TransformComponent>();
				const auto& enemySprite = entity.GetComponent<SpriteComponent>();
				auto& enemyRigidBody = entity.GetComponent<RigidBodyComponent>();
				const auto& enemyMovementType = entity.GetComponent<MovementTypeComponent>();

				//A script drives this entity for now: velocity has one owner at a time
				if (enemyAI.scriptControlsVelocity) {
					continue;
				}

				const glm::vec2 enemyCenter = EntityCenter(enemyTransform, enemySprite);
				const int enemyCol = tileMap.colAt(enemyCenter.x);
				const int enemyRow = tileMap.rowAt(enemyCenter.y);

				//Squared distance against a squared range: no sqrt per enemy per frame
				const double dx = playerCenter.x - enemyCenter.x;
				const double dy = playerCenter.y - enemyCenter.y;
				const double distanceSquared = dx * dx + dy * dy;

				const double range = enemyAI.isChasing ? enemyAI.detectionRange * chaseExitMultiplier : enemyAI.detectionRange;

				const bool wasChasing = enemyAI.isChasing;
				enemyAI.isChasing = distanceSquared < range * range;

				if (wasChasing != enemyAI.isChasing) {
					Logger::Log("Enemy " + std::to_string(entity.GetId()) + (enemyAI.isChasing ? " started chasing" : " stopped chasing"));
				}

				if (!enemyAI.isChasing) {
					enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
					enemyAI.path.clear();
					enemyAI.waypointIndex = 0;
					enemyAI.lastTargetCell = -1;
					continue;
				}

				enemyAI.repathTimer += deltaTime;

				//A player who stands still costs no searches at all
				const bool shouldSearch = playerCellIndex != enemyAI.lastTargetCell || enemyAI.repathTimer >= repathInterval;

				if (shouldSearch) {
					//Over budget: keep the old path and try again next frame
					if (searchesThisFrame >= maxSearchesPerFrame) {
						continue;
					}

					pathfinder.FindPath(tileMap, enemyCol, enemyRow, playerCol, playerRow, enemyMovementType.movementType, enemyAI.path);

					searchesThisFrame++;
					enemyAI.repathTimer = 0.0;
					enemyAI.lastTargetCell = playerCellIndex;
					enemyAI.waypointIndex = 0;

					if (logSearches) {
						Logger::Log("EntityId: " + std::to_string(entity.GetId()) + " Path size: " + std::to_string(enemyAI.path.size()));
					}
				}

				//Empty path: the goal could not be reached, so stand still
				if (enemyAI.waypointIndex >= static_cast<int>(enemyAI.path.size())) {
					enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
					continue;
				}

				//The last cell of the path is the player's own. Steering at the player rather
				//than at that tile's centre leaves the approach governed by the stop distance;
				//sharing the arrival radius there swallowed any smaller stop distance
				const bool isLastWaypoint = enemyAI.waypointIndex == static_cast<int>(enemyAI.path.size()) - 1;

				double targetX = playerCenter.x;
				double targetY = playerCenter.y;

				if (!isLastWaypoint) {
					const int cellIndex = enemyAI.path[enemyAI.waypointIndex];
					targetX = tileMap.IndexToCol(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
					targetY = tileMap.IndexToRow(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
				}

				const double wdx = targetX - enemyCenter.x;
				const double wdy = targetY - enemyCenter.y;
				const double waypointDistanceSquared = wdx * wdx + wdy * wdy;

				if (!isLastWaypoint) {
					const double arrivalRadius = std::max(
						tileWorldSize * arrivalRadiusInTiles,
						enemyAI.movementSpeed * deltaTime * arrivalFramesOfMovement
					);

					if (waypointDistanceSquared < arrivalRadius * arrivalRadius) {
						enemyAI.waypointIndex++;
					}
				}

				//Hulls touching, not centres coinciding, so a stop distance of 0 still looks right
				const double enemyHalfSize = enemySprite.width * enemyTransform.scale.x / 2.0;
				const double playerHalfSize = playerSprite.width * playerTransform.scale.x / 2.0;
				const double effectiveStopRange = std::max(enemyHalfSize + playerHalfSize, enemyAI.stopRange);

				if (distanceSquared < effectiveStopRange * effectiveStopRange) {
					enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
					continue;
				}

				const double length = std::sqrt(waypointDistanceSquared);

				//Standing exactly on the target: the direction would be undefined
				if (length < 0.001) {
					enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
					continue;
				}

				//Never step past the target, or the enemy oscillates around it
				double speedThisFrame = enemyAI.movementSpeed;
				if (speedThisFrame * deltaTime > length) {
					speedThisFrame = length / deltaTime;
				}

				//Velocity, never position: MovementSystem still applies terrain collision and speedPower
				enemyRigidBody.velocity = glm::vec2(wdx / length * speedThisFrame, wdy / length * speedThisFrame);
			}
		}


	private:
		//A chase ends further out than it starts, so an enemy on the boundary does not
		//flip state every frame
		static constexpr double chaseExitMultiplier = 1.2;

		//A path older than this is searched again even if the player has not changed cell
		static constexpr double repathInterval = 0.5;

		//Searching is the expensive part of a frame; enemies over budget wait rather than stall
		static constexpr int maxSearchesPerFrame = 2;

		//Waypoint reached: a quarter tile, but never less than a frame and a half of movement,
		//or a fast enemy steps over the point and orbits it
		static constexpr double arrivalRadiusInTiles = 0.25;
		static constexpr double arrivalFramesOfMovement = 1.5;

		static glm::vec2 EntityCenter(const TransformComponent& transform, const SpriteComponent& sprite) {
			return glm::vec2(
				transform.position.x + sprite.width * transform.scale.x / 2.0,
				transform.position.y + sprite.height * transform.scale.y / 2.0
			);
		}


};

#endif // !AISYSTEM_H

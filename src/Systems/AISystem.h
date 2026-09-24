#ifndef AISYSTEM_H
#define AISYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/AIComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Components/HealthComponent.h"
#include "../Pathfinding/Pathfinder.h"
#include "../TileMap/TileMap.h"
#include "../TileMap/MovementType.h"
#include "../Utils/EntityGeometry.h"
#include <string>
#include <algorithm>
#include <cmath>
#include <random>

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
					auto& entityAI = entity.GetComponent<AIComponent>();
					entity.GetComponent<RigidBodyComponent>().velocity = glm::vec2(0.0, 0.0);
					EnterAIState(entityAI, AIState::Patrol);
					
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

				//Squared distance of enemy to its spawn point (leash_range)
				const double spawndx = enemyAI.spawnPoint.x - enemyCenter.x;
				const double spawndy = enemyAI.spawnPoint.y - enemyCenter.y;
				const double spawnDistanceSquared = spawndx * spawndx + spawndy * spawndy;

				//To break chase player need to move 1.2* detection_range of enemy    or be further than it leash_range
				const double exitRange = enemyAI.detectionRange * chaseExitMultiplier;

				if (enemyAI.aggroCooldown > 0.0) {
					enemyAI.aggroCooldown -= deltaTime;
				}

				if (enemyAI.aggroTimer > 0.0) {
					enemyAI.aggroTimer -= deltaTime;
				}

				const AIState previousState = enemyAI.state;

				
				
				switch (enemyAI.state) {

					case AIState::Patrol:

						if (distanceSquared < enemyAI.detectionRange * enemyAI.detectionRange && enemyAI.aggroCooldown <= 0.0) {
							enemyAI.state = AIState::Chase;
						}

						break;
					
					case AIState::Chase:

						if (spawnDistanceSquared > enemyAI.leashRange * enemyAI.leashRange) {
							enemyAI.state = AIState::Return;
							Logger::Warn("Enemy: " + std::to_string(entity.GetId()) + " leashed, returing to spawn");
						}

						else if (distanceSquared > exitRange * exitRange && enemyAI.aggroTimer <= 0.0) {
							const bool nearHome = spawnDistanceSquared <= enemyAI.patrolRadius * enemyAI.patrolRadius;
							enemyAI.state = nearHome ? AIState::Patrol :AIState::Return;
							Logger::Warn("Enemy: " + std::to_string(entity.GetId()) + " lost the player, returing to spawn");
						}
						break;

					case AIState::Return:
						break;
				}


				if (previousState != enemyAI.state) {
					EnterAIState(enemyAI, enemyAI.state);
					Logger::Log("Enemy " + std::to_string(entity.GetId()) + (enemyAI.state == AIState::Chase ? " started chasing" : " stopped chasing"));
				}

				if (enemyAI.state == AIState::Patrol) {
					if (enemyAI.patrolRadius <= 0.0) {
						enemyRigidBody.velocity = glm::vec2{ 0.0, 0.0 };
						continue;
					}

					if (enemyAI.patrolPauseTimer > 0) {
						enemyAI.patrolPauseTimer -= deltaTime;
						enemyRigidBody.velocity = glm::vec2{ 0.0, 0.0 };
						continue;
					}

					const double tileWorldSize = tileMap.TileWorldSize();

					if (enemyAI.patrolTargetCell == -1) {
						const int radiusInCells = static_cast<int>(std::ceil(enemyAI.patrolRadius / tileWorldSize));
						const int spawnCol = tileMap.colAt(enemyAI.spawnPoint.x);
						const int spawnRow = tileMap.rowAt(enemyAI.spawnPoint.y);

						std::uniform_int_distribution<int> colDistribution(spawnCol - radiusInCells, spawnCol + radiusInCells);
						std::uniform_int_distribution<int> rowDistribution(spawnRow - radiusInCells, spawnRow + radiusInCells);

						//Rejection sampling: guess a cell near the spawn, keep the first usable one
						for (int attempt = 0; attempt < patrolPickAttempts; ++attempt) {

							const int col = colDistribution(randomEngine);
							const int row = rowDistribution(randomEngine);

							if (!tileMap.isInside(col, row)) {
								continue;
							}

							if (tileMap.isBlocked(col, row, enemyMovementType.movementType)) {
								continue;
							}

							//A circle, not the square the two distributions describe
							const int dcol = col - spawnCol;
							const int drow = row - spawnRow;

							if (dcol * dcol + drow * drow > radiusInCells * radiusInCells) {
								continue;
							}

							enemyAI.patrolTargetCell = tileMap.Index(col, row);
							enemyAI.lastTargetCell = -1;
							break;
						}

						//Nothing usable this frame (a cramped spawn): stand still and try again
						if (enemyAI.patrolTargetCell == -1) {
							Logger::Warn("Entity: " + std::to_string(entity.GetId()) + " no usable patrol cell near its spawn");
							enemyAI.patrolPauseTimer = enemyAI.patrolPause;
							enemyRigidBody.velocity = glm::vec2{ 0.0, 0.0 };
							continue;
						}
					}

					enemyAI.repathTimer += deltaTime;
					const int patrolCol = tileMap.IndexToCol(enemyAI.patrolTargetCell);
					const int patrolRow = tileMap.IndexToRow(enemyAI.patrolTargetCell);

					const bool shouldSearch = enemyAI.lastTargetCell != enemyAI.patrolTargetCell || enemyAI.repathTimer >= repathInterval;

					if (shouldSearch) {

						if (searchesThisFrame >= maxSearchesPerFrame) {
							continue;
						}

						const bool found = pathfinder.FindPath(tileMap, enemyCol, enemyRow, patrolCol, patrolRow, enemyMovementType.movementType, enemyAI.path);
						searchesThisFrame++;
						enemyAI.repathTimer = 0;
						enemyAI.lastTargetCell = enemyAI.patrolTargetCell;
						enemyAI.waypointIndex = 0;

						if (!found) {
							enemyAI.patrolTargetCell = -1;
							enemyAI.patrolPauseTimer = enemyAI.patrolPause;
							enemyRigidBody.velocity = glm::vec2{ 0.0,0.0 };
							continue;
						}
					}
					

					const double targetX = patrolCol * tileWorldSize + tileWorldSize / 2.0;
					const double targetY = patrolRow * tileWorldSize + tileWorldSize / 2.0;

					if (FollowPath(enemyAI, enemyRigidBody, enemyCenter, glm::vec2(targetX, targetY), tileWorldSize * arrivalRadiusInTiles, tileMap, deltaTime)) {
						enemyAI.patrolPauseTimer = enemyAI.patrolPause;
						enemyAI.patrolTargetCell = -1;
					}
					continue;
				}

				if (enemyAI.state == AIState::Return) {
					const int spawnCol = tileMap.colAt(enemyAI.spawnPoint.x);
					const int spawnRow = tileMap.rowAt(enemyAI.spawnPoint.y);
					const int spawnCellIndex = tileMap.Index(spawnCol, spawnRow);

					enemyAI.repathTimer += deltaTime;
					const bool shouldSearch = enemyAI.lastTargetCell != spawnCellIndex || enemyAI.repathTimer >= repathInterval;

					if (shouldSearch) {

						if (searchesThisFrame >= maxSearchesPerFrame) {
							continue;
						}


						const bool found = pathfinder.FindPath(tileMap, enemyCol, enemyRow, spawnCol, spawnRow, enemyMovementType.movementType, enemyAI.path);
						searchesThisFrame++;
						enemyAI.repathTimer = 0.0;
						enemyAI.lastTargetCell = spawnCellIndex;
						enemyAI.waypointIndex = 0;

						if (found) {
							enemyAI.failedSearchCount = 0;
						}

						else {
							enemyAI.failedSearchCount++;

							//No way home (pushed across water, or the spawn itself is blocked). A
							//returning enemy is immune, so leaving it stuck here would make it
							//permanently invulnerable: put it back where it belongs instead
							if (enemyAI.failedSearchCount >= maxFailedSearches) {
								auto& transformToSnap = entity.GetComponent<TransformComponent>();
								transformToSnap.position = glm::vec2(
									enemyAI.spawnPoint.x - enemySprite.width * enemyTransform.scale.x / 2.0,
									enemyAI.spawnPoint.y - enemySprite.height * enemyTransform.scale.y / 2.0
								);

								if (entity.HasComponent<HealthComponent>()) {
									auto& enemyHealth = entity.GetComponent<HealthComponent>();
									enemyHealth.healthPoints = enemyHealth.maxHealthPoints;
								}

								enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
								EnterAIState(enemyAI, AIState::Patrol);
								continue;
							}
						}
					}

					const double arrivalStop = tileMap.TileWorldSize() * arrivalRadiusInTiles;
					if (FollowPath(enemyAI, enemyRigidBody, enemyCenter, enemyAI.spawnPoint, arrivalStop, tileMap, deltaTime)) {

						if (entity.HasComponent<HealthComponent>()) {
							auto& enemyHealth = entity.GetComponent<HealthComponent>();
							enemyHealth.healthPoints = enemyHealth.maxHealthPoints;
						}
						EnterAIState(enemyAI, AIState::Patrol);
					}
					continue;
				}

				if (enemyAI.state == AIState::Chase) {
					enemyAI.repathTimer += deltaTime;

					//A player who stands still costs no searches at all
					const bool shouldSearch = playerCellIndex != enemyAI.lastTargetCell || enemyAI.repathTimer >= repathInterval;

					if (shouldSearch) {
						//Over budget: keep the old path and try again next frame
						if (searchesThisFrame >= maxSearchesPerFrame) {
							continue;
						}

						const bool found = pathfinder.FindPath(tileMap, enemyCol, enemyRow, playerCol, playerRow, enemyMovementType.movementType, enemyAI.path);

						searchesThisFrame++;
						enemyAI.repathTimer = 0.0;
						enemyAI.lastTargetCell = playerCellIndex;
						enemyAI.waypointIndex = 0;

						if (logSearches) {
							Logger::Log("EntityId: " + std::to_string(entity.GetId()) + " Path size: " + std::to_string(enemyAI.path.size()));
						}

						if (found) {
							enemyAI.failedSearchCount = 0;
						}
						else {
							enemyAI.failedSearchCount++;

							if (enemyAI.failedSearchCount >= maxFailedSearches) {
								Logger::Log("Enemy " + std::to_string(entity.GetId()) + " gave up: no path to the player");

								const bool nearHome = spawnDistanceSquared <= enemyAI.patrolRadius * enemyAI.patrolRadius;
								EnterAIState(enemyAI, nearHome ? AIState::Patrol : AIState::Return);
								enemyAI.aggroCooldown = aggroCooldownSeconds;
								enemyRigidBody.velocity = glm::vec2(0.0, 0.0);
								continue;
							}
						}

					}


					//Hulls touching, not centres coinciding, so a stop distance of 0 still looks right
					const double enemyHalfSize = enemySprite.width * enemyTransform.scale.x / 2.0;
					const double playerHalfSize = playerSprite.width * playerTransform.scale.x / 2.0;
					const double effectiveStopRange = std::max(enemyHalfSize + playerHalfSize, enemyAI.stopRange);

					FollowPath(enemyAI, enemyRigidBody, enemyCenter, playerCenter, effectiveStopRange, tileMap, deltaTime);
				}
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

		//Make data for random movement
		std::mt19937 randomEngine{ 1337 };

		//Number of chosing a new usable tile in patrol
		static constexpr int patrolPickAttempts = 10;

		//Unreachable target (the player standing on water): give up rather than stare
		static constexpr int maxFailedSearches = 3;

		static constexpr double aggroCooldownSeconds = 5.0;


		//Steers along ai.path; the last leg aims at finalTarget itself rather than its tile centre.
		//Returns true once within stopDistance of finalTarget, having stopped there
		bool FollowPath(AIComponent& ai, RigidBodyComponent& rigidBody,
			const glm::vec2& enemyCenter, const glm::vec2& finalTarget,
			double stopDistance, const TileMap& tileMap, double deltaTime) {

			const double tileWorldSize = tileMap.TileWorldSize();

			//Stopped at the target: checked first, so an empty path at the goal still counts as arrived
			const double tdx = finalTarget.x - enemyCenter.x;
			const double tdy = finalTarget.y - enemyCenter.y;
			if (tdx * tdx + tdy * tdy < stopDistance * stopDistance) {
				rigidBody.velocity = glm::vec2(0.0, 0.0);
				return true;                                                     
			}

			//Empty path: the goal could not be reached, so stand still
			if (ai.waypointIndex >= static_cast<int>(ai.path.size())) {
				rigidBody.velocity = glm::vec2(0.0, 0.0);
				return false;                                                         
			}

			//On the last leg, aim at the target itself rather than its tile centre, so the
			//approach ends on the stop distance and not on the arrival radius
			const bool isLastWaypoint = ai.waypointIndex == static_cast<int>(ai.path.size()) - 1;

			double targetX = finalTarget.x;                                            
			double targetY = finalTarget.y;

			if (!isLastWaypoint) {
				const int cellIndex = ai.path[ai.waypointIndex];
				targetX = tileMap.IndexToCol(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
				targetY = tileMap.IndexToRow(cellIndex) * tileWorldSize + tileWorldSize / 2.0;
			}

			const double wdx = targetX - enemyCenter.x;
			const double wdy = targetY - enemyCenter.y;
			const double waypointDistanceSquared = wdx * wdx + wdy * wdy;

			if (!isLastWaypoint) {
				const double arrivalRadius = std::max(
					tileWorldSize * arrivalRadiusInTiles,
					ai.movementSpeed * deltaTime * arrivalFramesOfMovement
				);

				if (waypointDistanceSquared < arrivalRadius * arrivalRadius) {
					ai.waypointIndex++;
				}
			}

			const double length = std::sqrt(waypointDistanceSquared);

			//Standing exactly on the target: the direction would be undefined
			if (length < 0.001) {
				rigidBody.velocity = glm::vec2(0.0, 0.0);
				return false;
			}

			//Never step past the target, or the enemy oscillates around it
			double speedThisFrame = ai.movementSpeed;
			if (speedThisFrame * deltaTime > length && deltaTime > 0.0) {
				speedThisFrame = length / deltaTime;
			}

			//Velocity, never position: MovementSystem still applies terrain collision and speedPower
			rigidBody.velocity = glm::vec2(wdx / length * speedThisFrame, wdy / length * speedThisFrame);
			return false;
		}



};

#endif // !AISYSTEM_H

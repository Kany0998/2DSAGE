#ifndef ACTIVATIONSYSTEM_H
#define ACTIVATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Game/SimulationSettings.h"
#include "../TileMap/TileMap.h"
#include "../Components/ActiveComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/HealthComponent.h"
#include "../Utils/EntityGeometry.h"
#include "../Logger/Logger.h"
#include <vector>
#include <glm/glm.hpp>

class ActivationSystem {
	public:
		ActivationSystem() = default;

		void Update(Registry& registry, double deltaTime, SimulationSettings& simulationSettings, TileMap& tileMap) {
			timer += deltaTime;
			if (timer <= simulationSettings.checksIntervalSeconds) {
				return;
			}

			timer = 0.0;

			std::vector<Entity> enemies = registry.GetEntitiesByGroup("enemies");
			if (!simulationSettings.enable) {
				for (auto& enemy : enemies) {
					if (!enemy.HasComponent<ActiveComponent>()) {
						enemy.AddComponent<ActiveComponent>();
					}
				}
				return;
			}

			if (!registry.HasEntityWithTag("player")) {
				return;
			}

			Entity player = registry.GetEntityByTag("player");

			if (!player.HasComponent<TransformComponent>() || !player.HasComponent<SpriteComponent>()) {
				return;
			}

			const auto& playerTransform = player.GetComponent<TransformComponent>();
			const auto& playerSprite = player.GetComponent<SpriteComponent>();

			const glm::vec2 playerCenter = EntityCenter(playerTransform, playerSprite);
			const double tileWorldSize = tileMap.TileWorldSize();

			const double activationRange = simulationSettings.activateTilesRadius * tileWorldSize;
			const double deactivationRange = simulationSettings.deactivateTilesRadius * tileWorldSize;

			const double activationRangeSq = activationRange * activationRange;
			const double deactivationRangeSq = deactivationRange * deactivationRange;

			for (auto& enemy : enemies) {

				if (!enemy.HasComponent<TransformComponent>() || !enemy.HasComponent<SpriteComponent>()) {
					continue;
				}
				const glm::vec2 enemyCenter = EntityCenter(enemy.GetComponent<TransformComponent>(), enemy.GetComponent<SpriteComponent>());
				const glm::vec2 distance = playerCenter - enemyCenter;
				const double distanceSq = distance.x * distance.x + distance.y * distance.y;
				const bool active = enemy.HasComponent<ActiveComponent>();

				if (!active && distanceSq <= activationRangeSq ) {
					enemy.AddComponent<ActiveComponent>();
					if (enemy.HasComponent<AIComponent>()) {
						auto& enemyAI = enemy.GetComponent<AIComponent>();
						const double spawndx = enemyAI.spawnPoint.x - enemyCenter.x;
						const double spawndy = enemyAI.spawnPoint.y - enemyCenter.y;
						const double spawnDistanceSquared = spawndx * spawndx + spawndy * spawndy;

						const bool nearHome = spawnDistanceSquared <= enemyAI.patrolRadius * enemyAI.patrolRadius;
						
						const bool fullHealth = !enemy.HasComponent<HealthComponent>() || enemy.GetComponent<HealthComponent>().healthPoints == enemy.GetComponent<HealthComponent>().maxHealthPoints;

						EnterAIState(enemyAI, (nearHome && fullHealth) ? AIState::Patrol : AIState::Return);
					}
				}

				if (active && distanceSq >= deactivationRangeSq) {
					enemy.RemoveComponent<ActiveComponent>();
					if (enemy.HasComponent<RigidBodyComponent>()) {
						enemy.GetComponent<RigidBodyComponent>().velocity = glm::vec2(0.0, 0.0);
					}
				}
			}
		}

	private:

		double timer = 0.0;	//Seconds since last check
};

#endif // !ACTIVATIONSYSTEM_H

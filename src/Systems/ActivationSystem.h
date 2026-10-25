#ifndef ACTIVATIONSYSTEM_H
#define ACTIVATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Game/SimulationSettings.h"
#include "../TileMap/TileMap.h"
#include "../Components/ActiveComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
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

			const double& activactionRange = simulationSettings.activateTilesRadius * tileWorldSize;
			const double& deactivactionRange = simulationSettings.deactivateTilesRadius * tileWorldSize;

			const double activactionRangeSq = activactionRange * activactionRange;
			const double deactivactionRangeSq = deactivactionRange * deactivactionRange;

			for (auto& enemy : enemies) {

				if (!enemy.HasComponent<TransformComponent>() || !enemy.HasComponent<SpriteComponent>()) {
					continue;
				}
				const glm::vec2 enemyCenter = EntityCenter(enemy.GetComponent<TransformComponent>(), enemy.GetComponent<SpriteComponent>());
				const glm::vec2 distance = playerCenter - enemyCenter;
				const double distanceSq = distance.x * distance.x + distance.y * distance.y;
				const bool active = enemy.HasComponent<ActiveComponent>();

				if (!active && distanceSq <= activactionRangeSq ) {
					enemy.AddComponent<ActiveComponent>();
				}

				if (active && distanceSq >= deactivactionRangeSq) {
					enemy.RemoveComponent<ActiveComponent>();
				}
			}
		}

	private:

		double timer = 0.0;	//Seconds since last check
};

#endif // !ACTIVATIONSYSTEM_H

#ifndef HEALTHREGENERATIONSYSTEM_H
#define HEALTHREGENERATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/HealthComponent.h"
#include "../Components/AttributesComponent.h"

class HealthRegenerationSystem
{
public:
	HealthRegenerationSystem() = default;

	void Update(Registry& registry, float deltaTime) {
		for (auto rawEntity : registry.Raw().view<HealthComponent, AttributesComponent>()) {
			Entity entity(rawEntity, &registry);
			auto& health = entity.GetComponent<HealthComponent>();
			auto attributes = entity.GetComponent<AttributesComponent>();

			if (health.healthPoints < health.maxHealthPoints) {
				health.healthRegenAccumulator += (1 + attributes.vitalityPower / 10.0f) * deltaTime;
			}
			while (health.healthRegenAccumulator >= 1.0f) {
				health.healthRegenAccumulator -= 1.0f;
				health.healthPoints += 1;
			}
			if (health.healthPoints >= health.maxHealthPoints) {
				health.healthPoints = health.maxHealthPoints;
			}
		}
	}


};

#endif
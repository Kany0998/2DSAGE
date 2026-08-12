#ifndef MANAREGENERATIONSYSTEM_H
#define MANAREGENERATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/ManaComponent.h"
#include "../Components/AttributesComponent.h"

class ManaRegenerationSystem
{
public:
	ManaRegenerationSystem() = default;

	void Update(Registry &registry, float deltaTime){
		for (auto rawEntity : registry.Raw().view<ManaComponent, AttributesComponent>()) {
			Entity entity(rawEntity, &registry);
			auto& mana = entity.GetComponent<ManaComponent>();
			auto attributes = entity.GetComponent<AttributesComponent>();

			if (mana.manaPoints < mana.maxManaPoints) {
				mana.manaRegenAccumulator += (1 + attributes.wisdomPower / 10.0f) * deltaTime;
			}
			while (mana.manaRegenAccumulator >= 1.0f) {
				mana.manaRegenAccumulator -= 1.0f;
				mana.manaPoints += 1;
			}
			if(mana.manaPoints >= mana.maxManaPoints) {
				mana.manaPoints = mana.maxManaPoints;
			}
		}
	}

	
};

#endif
#ifndef LOOTSYSTEM_H
#define LOOTSYSTEM_H


#include "../ECS/ECS.h"
#include "../Events/LootDroppedEvent.h"
#include "../Events/EntityKilledEvent.h"
#include "../Events/ItemDroppedEvent.h"
#include "../EventBus/EventBus.h"
#include "../Loot/LootDatabase.h"
#include "../Components/LootDropComponent.h"
#include "../Components/LootBagComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Utils/EntityGeometry.h"
#include "../Inventory/ItemDatabase.h"
#include "../AssetStore/AssetStore.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <random>

class LootSystem {
	public:
		LootSystem(Registry& registry, const LootDatabase& lootDatabase, const ItemDatabase& itemDatabase, AssetStore& assetStore, std::unique_ptr<EventBus>& eventBus)
			: registry(registry), lootDatabase(lootDatabase), itemDatabase(itemDatabase), assetStore(assetStore), eventBus(eventBus) {}
		



		void SubscribeToEvents()
		{
			eventBus->SubscribeToEvent<EntityKilledEvent>(this, &LootSystem::OnEntityKilled);
			eventBus->SubscribeToEvent<ItemDroppedEvent>(this, &LootSystem::OnItemDropped);
		}



		void Update() {
			for (const PendingDrop&  drop : pendingDrops) {

				const auto tier = lootDatabase.FindBag(drop.tierId);

				if (tier == nullptr) {
					continue;
				}

				int width = 8;
				int height = 8;

				//size from the bag's texture; a missing texture keeps 8x8 (the bag exists but is invisible)
				SDL_Texture* texture = assetStore.GetTexture(tier->texture);
				if (texture == nullptr) {
					Logger::Warn("loot: bag '" + tier->id + "' uses texture '" + tier->texture + "', which is not in assets.lua - the bag is invisible");
				}

				else {
					SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
				}

				const float scale = static_cast<float>(lootDatabase.BagScale());
				const glm::vec2 corner = drop.position - glm::vec2(width * scale / 2.0f, height * scale / 2.0f);

				Entity bag = registry.CreateEntity();
				bag.Group("lootbags");
				bag.AddComponent<TransformComponent>(corner, glm::vec2(scale, scale));
				bag.AddComponent<SpriteComponent>(tier->texture, width, height, 1);
				bag.AddComponent<LootBagComponent>(tier->id, drop.items, tier->lifetimeMs);
				eventBus->EmitEvent<LootDroppedEvent>(drop.position, tier->id, drop.fromKill);
			}

			pendingDrops.clear();
			std::vector<Entity> bags;
			for (auto rawEntity : registry.Raw().view<LootBagComponent, SpriteComponent>()) {
				bags.emplace_back(rawEntity, &registry);
			}

			for (auto entity : bags) {
				auto& bagSprite = entity.GetComponent<SpriteComponent>();
				auto& bagLoot = entity.GetComponent<LootBagComponent>();

				const Uint32 age = SDL_GetTicks() - bagLoot.spawnTime;
				if (age >= static_cast<Uint32>(bagLoot.lifetimeMs) || bagLoot.items.empty()) {
					entity.Kill();
					continue;
				}

				if (static_cast<Uint32>(bagLoot.lifetimeMs) - age < static_cast<Uint32>(lootDatabase.BlinkMs())) {
					if ((age / 250) % 2 == 0) {
						bagSprite.brightness = 110;
					}
					else {
						bagSprite.brightness = 255;
					}
				}
			}
		}



		void OnEntityKilled(EntityKilledEvent& event) {
			//Check if entity can drop something
			if (!event.victim.HasComponent<LootDropComponent>()) {
				return;
			}

			const std::string tableId = event.victim.GetComponent<LootDropComponent>().tableId;
			const std::vector<LootEntry>* table = lootDatabase.FindTable(tableId);

			if (table == nullptr) {
				return;
			}

			std::uniform_real_distribution<float> roll(0.0f, 1.0f);		//the dice, created once
			std::vector<std::string> dropped;							//what this kill drops

			for (const LootEntry& entry : *table) {

				//check if rng is smaller and when it is we add item to drop list for example
				//black chest around 0.02 we roll 0.01 we add it up if roll is higer we skip
				if (roll(rng) < entry.chance) {
					dropped.push_back(entry.itemId);
				}
			}

			if (dropped.empty()) {
				return;
			}

			const BagTier* best = nullptr;

			for (const std::string& itemId : dropped) {

				const ItemDefinition* item = itemDatabase.Find(itemId);		//item data
				//Should never happen but it is still better to check it
				if (item == nullptr) {
					continue;
				}

				const BagTier* bag = lootDatabase.BagFor(*item);		//bag item drops in
				if (bag == nullptr) {
					continue;				//no tiers
				}

				//assign higest rank bag to best
				if (best == nullptr || best->rank < bag->rank) {
					best = bag;
				}

			}

			if (best == nullptr) {
				return;
			}

			if (!event.victim.HasComponent<TransformComponent>()) {
				return;		//nowhere to put the bag
			}

			const auto& victimTransform = event.victim.GetComponent<TransformComponent>();
			glm::vec2 bagPosition = victimTransform.position;		//fallback corner
			if (event.victim.HasComponent<SpriteComponent>()) {
				const auto& victimSprite = event.victim.GetComponent<SpriteComponent>();
				bagPosition = EntityCenter(victimTransform, victimSprite);
			}
			pendingDrops.push_back({ bagPosition, best->id, dropped });
		}

		void OnItemDropped(ItemDroppedEvent& event) {

			const ItemDefinition* item = itemDatabase.Find(event.itemId);
			if (item == nullptr) {
				Logger::Warn("loot: item " + event.itemId + " has unknown id");
				return;
			}

			const BagTier* bag = lootDatabase.PlayerDropBag();
			if (bag == nullptr) {
				bag = lootDatabase.BagFor(*item);
			}

			pendingDrops.push_back({ event.position, bag->id, { event.itemId }, false });
		}



	private:


		struct PendingDrop {
			glm::vec2 position;
			std::string tierId;
			std::vector<std::string> items;
			bool fromKill = true;
		};

		std::vector<PendingDrop> pendingDrops;
		std::mt19937 rng{ std::random_device{}() };

		Registry& registry;
		const LootDatabase& lootDatabase;
		const ItemDatabase& itemDatabase;
		AssetStore& assetStore;
		std::unique_ptr<EventBus>& eventBus;
};


#endif // !LOOTSYSTEM_H

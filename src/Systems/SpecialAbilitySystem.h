#ifndef SPECIALABILITYSYSTEM_H
#define SPECIALABILITYSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/AbilityCastEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/ManaComponent.h"
#include "../Components/EquipmentComponent.h"
#include "../Components/AbilityStateComponent.h"
#include "../Inventory/ItemDatabase.h"
#include "../Abilities/AbilityDatabase.h"
#include "../Logger/Logger.h"
#include "../Components/CameraHollderComponent.h"
#include <cmath>
#include <SDL.h>
#include <glm/glm.hpp>

// SPACE casts the Lua ability of the item in the caster's Ability slot at the mouse's world position.
// C++ owns the shared rules (cooldown, mana, protected call); the ability's on_cast decides what happens.
// Mana and cooldown are paid only when on_cast succeeds.
class SpecialAbilitySystem{
	public:
		// Needs registry + camera kept alive for OnKeyPressed, since that's an
		// event-callback with a fixed (KeyPressedEvent&) signature and can't
		// receive them as call parameters like Update() does elsewhere.
		SpecialAbilitySystem(Registry& registry, const SDL_Rect& camera, const ItemDatabase& itemDatabase, const AbilityDatabase& abilityDatabase, std::unique_ptr<EventBus>& eventBus) 
			: registry(registry), camera(camera), itemDatabase(itemDatabase), abilityDatabase(abilityDatabase), eventBus(eventBus) {}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus){
			eventBus->SubscribeToEvent<KeyPressedEvent>(this, &SpecialAbilitySystem::OnKeyPressed);
		}

		void OnKeyPressed(KeyPressedEvent& event){
			if (event.symbol != SDLK_SPACE){

				return;
			}

			// Snapshot first: the lua on_cast creates new entities with a TransformComponent,
			// which can grow/reallocate that component's pool. Doing that while still
			// iterating a live view over TransformComponent is unsafe - especially here,
			// however many entities on_cast creates
			std::vector<Entity> casters;
			for (auto rawEntity : registry.Raw().view<ManaComponent, TransformComponent, EquipmentComponent, CameraHollderComponent>()){
				casters.emplace_back(rawEntity, &registry);
			}

			for (auto entity : casters){
				const auto& equipment = entity.GetComponent<EquipmentComponent>().slots;
				std::string itemId;

				for (const auto& slot : equipment) {
					if (slot.accepts == "special_ability" && !slot.itemId.empty()) {
						itemId = slot.itemId;
						break;
					}
				}

				//No ability equiped continue
				if (itemId.empty()) {
					continue;
				}

				//If item is null we skip
				const ItemDefinition* item = itemDatabase.Find(itemId);
				if (item == nullptr) {
					continue;
				}

				const AbilityDefinition* ability = abilityDatabase.Find(item->ability);
				const sol::table* stats = abilityDatabase.StatsForItem(itemId);
				if (ability == nullptr || stats == nullptr) {
					continue;		//Item do not contain valid ability
				}

				if (!entity.HasComponent<AbilityStateComponent>()) {
					entity.AddComponent<AbilityStateComponent>();
				}

				const Uint32 cooldown = static_cast<Uint32>(std::lround((*stats)["cooldown_seconds"].get_or(0.0) * 1000));
				const Uint32 lastCastTime = entity.GetComponent<AbilityStateComponent>().lastCastTime;

				if (SDL_GetTicks() - lastCastTime < cooldown) {
					continue;
				}

				const int manaCost = static_cast<int>(std::lround((*stats)["mana_cost"].get_or(0.0)));
				if (entity.GetComponent<ManaComponent>().manaPoints < manaCost) {
					continue;
				}

				const std::string castSound = (*stats)["cast_sound"].get_or(std::string(""));

				int mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);
				glm::vec2 target(mouseX + camera.x, mouseY + camera.y);

				//Call lua on_cast
				sol::protected_function_result result = ability->onCast(entity, target.x, target.y, *stats);
				if (!result.valid()) {
					sol::error err = result;
					Logger::Err("Ability: " + ability->id + ": " + err.what());
					continue;
				}
				auto& manaComp = entity.GetComponent<ManaComponent>();
				auto& state = entity.GetComponent<AbilityStateComponent>();

				manaComp.manaPoints -= manaCost;

				//only pay and start the cooldown once a cast actually went through - a cast refused
				//for lack of mana, or one whose on_cast failed in Lua, shouldn't cost anything
				state.lastCastTime = SDL_GetTicks();

				eventBus->EmitEvent<AbilityCastEvent>(entity, castSound);
			}
		}

	private:
		Registry& registry;
		const SDL_Rect& camera;
		const ItemDatabase& itemDatabase;
		const AbilityDatabase& abilityDatabase;
		std::unique_ptr<EventBus>& eventBus;
		
};

#endif

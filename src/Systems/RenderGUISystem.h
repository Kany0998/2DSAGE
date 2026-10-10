#ifndef RENDERGUISYSTEM_H
#define RENDERGUISYSTEM_H

#include "../ECS/ECS.h"
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ProgressionComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/BackpackComponent.h"
#include "../Components/EquipmentComponent.h"
#include "../TileMap/TileMap.h"
#include "../TileMap/MovementType.h"
#include "../Game/Controls.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Inventory/InventoryUISettings.h"
#include "../Inventory/ItemDatabase.h"
#include "../Inventory/InventoryRules.h"
#include "../AssetStore/AssetStore.h"
#include <algorithm>
#include <cmath>


class RenderGUISystem
{
	public:
		RenderGUISystem(const Controls& controls, const InventoryUISettings& inventoryUI,
			const ItemDatabase& itemDatabase, AssetStore& assetStore) : controls(controls),
			inventoryUI(inventoryUI), itemDatabase(itemDatabase), assetStore(assetStore){}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
		{
			eventBus->SubscribeToEvent<KeyPressedEvent>(this, &RenderGUISystem::OnKeyPressed);
		}

		void OnKeyPressed(KeyPressedEvent& event)
		{
			if (event.isRepeat) return;                          //holding a key must not flicker the window
			if (ImGui::GetIO().WantCaptureKeyboard) return;      //typing into an ImGui field is not a toggle

			if (event.symbol == controls.equipment) showEquipment = !showEquipment;
			if (event.symbol == controls.backpack)  showBackpack = !showBackpack;
			if (event.symbol == controls.character) showCharacter = !showCharacter;
		}

		void Update(Registry& registry, const SDL_Rect& camera, bool isDebug, const TileMap& tilemap)
		{
			//Render GUI elements here
			ImGui::NewFrame();

			
			if (isDebug) {
				if (ImGui::Begin("Spawn enemies"))
				{
					//Input for enemy to hold inputs
					static int positionX = 0;
					static int positionY = 0;
					static int scaleX = 1;
					static int scaleY = 1;
					static int velocityX = 0;
					static int velocityY = 0;
					static int health = 100;
					static float rotate = 0.0f;
					static float projectileAngle = 0.0f;
					static float projectileSpeed = 100.0f;
					static int projectileRate = 10;
					static int projectileDuration = 10;
					static int projectileDamage = 10;
					const char* sprites[] = { "tank-texture", "truck-texture", "chopper-texture" };
					static int selectedSpriteIndex = 0;

					//sprite selection
					if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Combo("texture id", &selectedSpriteIndex, sprites, IM_ARRAYSIZE(sprites));
					}
					ImGui::Spacing();

					//Transform inputs
					if (ImGui::CollapsingHeader("Transfrom", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::InputInt("enemy x position", &positionX);
						ImGui::InputInt("enemy y position", &positionY);
						ImGui::SliderInt("scale x", &scaleX, 1, 10);
						ImGui::SliderInt("scale y", &scaleY, 1, 10);
						ImGui::SliderAngle("rotation (degrees)", &rotate, 0, 360);
					}
					ImGui::Spacing();

					//RigidBody inputs
					if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::InputInt("velocity x", &velocityX);
						ImGui::InputInt("velocity y", &velocityY);
					}
					ImGui::Spacing();

					//Projectile Emitter inputs
					if (ImGui::CollapsingHeader("Projectile emitter", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::SliderAngle("angle (degrees)", &projectileAngle, 0, 360);
						ImGui::SliderFloat("speed (px/sec)", &projectileSpeed, 10, 500);
						ImGui::InputInt("emission rate (sec)", &projectileRate);
						ImGui::InputInt("projectile duration (sec)", &projectileDuration);
						ImGui::SliderInt("projectile damage (%)", &projectileDamage, 0, 100);
					}
					ImGui::Spacing();

					if (ImGui::CollapsingHeader("Health", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::SliderInt("%", &health, 0, 100);
					}
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					if (ImGui::Button("Create new enemy"))
					{
						//Emit event on button click
						Entity enemy = registry.CreateEntity();
						enemy.Group("enemies");
						enemy.AddComponent<TransformComponent>(glm::vec2(positionX, positionY), glm::vec2(scaleX, scaleY), glm::degrees(rotate));
						enemy.AddComponent<RigidBodyComponent>(glm::vec2(velocityX, velocityY));
						enemy.AddComponent<SpriteComponent>(sprites[selectedSpriteIndex], 32, 32, 3);
						enemy.AddComponent<BoxColliderComponent>(32, 32);
						double projectileVelX = cos(projectileAngle) * projectileSpeed;
						double projectileVelY = sin(projectileAngle) * projectileSpeed;
						enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(projectileVelX, projectileVelY), projectileRate * 1000, projectileDuration * 1000, projectileDamage, false);
						enemy.AddComponent<HealthComponent>(health);

						//Reset inputs after creation of enemy
						positionX = 0;
						positionY = 0;
						rotate = 0;
						projectileAngle = 0;
						scaleX = scaleY = 1;
						projectileRate = projectileDuration = projectileDamage = 10;
						projectileSpeed = 100;
						health = 100;
					}

					
				}
				ImGui::End();


				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav;
				ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
				ImGui::SetNextWindowBgAlpha(0.5f);
				if (ImGui::Begin("Map Coordinates", NULL, windowFlags))
				{
					double worldX = ImGui::GetIO().MousePos.x + camera.x;
					double worldY = ImGui::GetIO().MousePos.y + camera.y;

					int col = tilemap.colAt(worldX);
					int row = tilemap.rowAt(worldY);
					int tileId = tilemap.tileAt(col, row);

					bool isBlockedGround = tilemap.isBlocked(col, row, MovementType_Ground);
					bool isBlockedFlying = tilemap.isBlocked(col, row, MovementType_Flying);

					double speedMultiplier = tilemap.SpeedMultiplierAt(col, row);
					double friction = tilemap.FrictionAt(col, row);
					double acceleration = tilemap.AccelerationAt(col, row);


					ImGui::Text("Map coordinates(x=%.1f, y=%.1f)",worldX,worldY);
					ImGui::Text("Map tile (col=%d, row=%d, id=%d)", col, row, tileId);
					ImGui::Text("Map tile is blocked (ground): %s", isBlockedGround ? "Yes" : "No");
					ImGui::Text("Map tile is blocked (flying): %s", isBlockedFlying ? "Yes" : "No");
					ImGui::Text("Map tile speed multiplier: multi=%.3f", speedMultiplier);
					ImGui::Text("Map tile friciton: multi=%.2f", friction);
					ImGui::Text("Map tile acceleration: multi=%.2f", acceleration);
				}
				ImGui::End();
			}

			if (registry.HasEntityWithTag("player")) {
				Entity player = registry.GetEntityByTag("player");

				if (player.IsAlive() && player.HasComponent<ProgressionComponent>() && player.HasComponent<AttributesComponent>()) {

					//both non-const: spending a point writes to each of them
					auto& progression = player.GetComponent<ProgressionComponent>();
					auto& attributes = player.GetComponent<AttributesComponent>();

					if (showCharacter) {
						ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_FirstUseEver);

						if (ImGui::Begin("Character", &showCharacter)) {
							ImGui::Text("Level: %d", progression.currentLevel);
							ImGui::Text("%d / %d exp", progression.currentExperience, progression.experienceToNextLevel);
							ImGui::Text("Skill points: %d", progression.unspentSkillPoints);

							ImGui::Separator();

							// The stat order comes from the SkillStat enum, so the rows, the labels and the
							// reset below all index the same way. The labels only have to follow that order.



							for (int i = 0; i < Skill_Count; i++)
							{
								StatRow(StatLabels[i], attributes, static_cast<SkillStat>(i), progression);
							}

							ImGui::Separator();


							int totalSpent = 0;
							for (int i = 0; i < Skill_Count; i++)
							{
								totalSpent += progression.spentPoints[i];
							}

							const bool canReset = totalSpent > 0;
							if (!canReset) { ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f); }

							if (ImGui::Button("Reset points") && canReset)
							{
								for (int i = 0; i < Skill_Count; i++)
								{
									attributes.base[i] -= progression.spentPoints[i];
									progression.unspentSkillPoints += progression.spentPoints[i];
									progression.spentPoints[i] = 0;
								}
							}

							if (!canReset) { ImGui::PopStyleVar(); }

							ImGui::SameLine();
							ImGui::Text("(%d invested)", totalSpent);
						}
						ImGui::End();
					}

					if (showEquipment) {
						if (ImGui::Begin("Equipment", &showEquipment, ImGuiWindowFlags_AlwaysAutoResize)) {
							if (player.HasComponent<EquipmentComponent>() && player.HasComponent<BackpackComponent>() && player.HasComponent<AttributesComponent>()) {
								auto& equipment = player.GetComponent<EquipmentComponent>();
								auto& backpack = player.GetComponent<BackpackComponent>();
								auto& attributes = player.GetComponent<AttributesComponent>();

								//paper doll: every placed slot sits on its Lua grid cell, measured from here
								const ImVec2 origin = ImGui::GetCursorPos();
								const float cell = inventoryUI.SlotPixels() + 6.0f;		//6px gap
								int lowestRow = -1;

								for (int i = 0; i < static_cast<int>(equipment.slots.size()); i++) {
									const EquipmentSlot& slot = equipment.slots[i];
									const ItemDefinition* item = slot.itemId.empty() ? nullptr : itemDatabase.Find(slot.itemId);

									ImGui::PushID(i);
									ImGui::SetCursorPos(ImVec2(origin.x + slot.column * cell, origin.y + slot.row * cell));
									DrawItemIcon(item);

									if (item != nullptr && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
										std::string message;
										if (!InventoryRules::Unequip(i, backpack, equipment, attributes, itemDatabase, message)) {
											ShowInventoryMessage(message);
										}
									}

									if (ImGui::IsItemHovered()) {
										DrawSlotTooltip(slot, item);
									}

									ImGui::PopID();

									lowestRow = std::max(lowestRow, slot.row);
								}
								//continue under the paper doll: the message, then the stats
								ImGui::SetCursorPos(ImVec2(origin.x, origin.y + (lowestRow + 1) * cell));

								if (!inventoryMessage.empty() && SDL_GetTicks() - inventoryMessageTime < 3000) {
									ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", inventoryMessage.c_str());
								}

								ImGui::Separator();
								for (int i = 0; i < Skill_Count; i++) {
									StatText(static_cast<SkillStat>(i), attributes);
								}
							}
						}
						ImGui::End();
					}
					if (showBackpack) {
						if (ImGui::Begin("Backpack", &showBackpack)) { 
							if (player.HasComponent<BackpackComponent>()) {
								const auto& backpack = player.GetComponent<BackpackComponent>();
								for (int i = 0; i < static_cast<int>(backpack.items.size()); i++) {
									if (i > 0) ImGui::SameLine();
									DrawItemIcon(itemDatabase.Find(backpack.items[i]));
								}
							}
						}
						ImGui::End();
					}

				}
			}

			ImGui::Render();
			ImGuiSDL::Render(ImGui::GetDrawData());
		}

	private:
		// One row of the character sheet. Takes the attributes and which stat it shows, so it can
		// display the effective value and spend points into the base only. When per-class caps
		// arrive, the spend rule goes here rather than into six separate button conditions.
		void StatRow(const char* label, AttributesComponent& attributes, SkillStat stat, ProgressionComponent& progression)
		{
			// All six buttons are labelled "+", and ImGui derives widget identity from
			// the label - without a unique id pushed around each row they would all be
			// the same widget. The stat names are distinct, so they serve as the id.
			ImGui::PushID(label);

			int& spent = progression.spentPoints[stat];
			const bool canSpend = progression.unspentSkillPoints > 0;

			// ImGui 1.79 has no BeginDisabled(), so the row is dimmed by hand
			if (!canSpend) { ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f); }

			StatText(stat, attributes);
			ImGui::SameLine();
			ImGui::Text("(+%d)", spent);
			ImGui::SameLine();

			// Button() is called unconditionally and the result filtered afterwards.
			// Testing canSpend first would short-circuit the call, and the buttons
			// would vanish rather than grey out whenever the pool hit zero.
			if (ImGui::Button("+") && canSpend)
			{
				progression.unspentSkillPoints--;
				attributes.base[stat]++;
				spent++;
			}

			if (!canSpend) { ImGui::PopStyleVar(); }

			ImGui::PopID();
		}

		//One stat as text: name, effective value, and the base/item split when an item adds to it.
		//Shared by the character sheet (which adds spending) and the equipment window (read-only).
		void StatText(SkillStat stat, const AttributesComponent& attributes)
		{
			const int bonus = attributes.bonus[stat];
			ImGui::Text("%-10s %6d", StatLabels[stat], attributes.Effective(stat));
			if (bonus != 0) {
				ImGui::SameLine();
				ImGui::Text("[base %d, item %+d]", attributes.base[stat], bonus);
			}
		}


		//Draws one inventory slot at the cursor: a frame, then the item's icon fitted inside it.
		//Always takes exactly one slot of space, so rows and grids line up; item may be nullptr (empty slot).
		void DrawItemIcon(const ItemDefinition* item){
			
			//layout position where ImGui places the next widget (not the mouse)
			const float slot = inventoryUI.SlotPixels();
			const ImVec2 corner = ImGui::GetCursorScreenPos();
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			
			//frame: dark background, lighter border
			const ImVec2 farCorner(corner.x + slot, corner.y + slot);
			drawList->AddRectFilled(corner, farCorner, IM_COL32(30, 30, 30, 255));
			drawList->AddRect(corner, farCorner, IM_COL32(90, 90, 90, 255));
			
			//icon: fitted without stretching, whole-number scale when it fits, centred
			SDL_Texture* texture = nullptr;
			if (item != nullptr && !item->icon.empty()) {
				texture = assetStore.GetTexture(item->icon);
			}

			if (texture != nullptr) {
				int width = 0;
				int height = 0;
				SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

				if (width > 0 && height > 0) {
					float fit = std::min(slot / width, slot / height);

					if (fit >= 1.0f) {
						fit = std::floor(fit);
					}
					const float drawWidth = width * fit;
					const float drawHeigth = height * fit;

					const ImVec2 iconMin(corner.x + (slot - drawWidth) / 2.0f, corner.y + (slot - drawHeigth) / 2.0f);
					const ImVec2 iconMax(iconMin.x + drawWidth, iconMin.y + drawHeigth);
					drawList->AddImage(texture, iconMin, iconMax);
				}
			}
			//reserve the slot's space: the next widget goes after it, and hover/click cover the whole slot
			ImGui::Dummy(ImVec2(slot, slot));
		}

		//Shows a refusal from Equip/Unequip in the inventory windows for a few seconds
		void ShowInventoryMessage(const std::string& message){
			inventoryMessage = message;
			inventoryMessageTime = SDL_GetTicks();
		}

		//Hover tooltip for an equipment slot: the slot's name, then the item's details, or that it is empty
		void DrawSlotTooltip(const EquipmentSlot& slot, const ItemDefinition* item) {
			ImGui::BeginTooltip();
			ImGui::TextDisabled("%s", slot.name.c_str());	//slot name, greyed out on top

			if (item == nullptr) {
				ImGui::Text(slot.accepts == "special_ability" ? "No special ability" : "Empty");
			}

			else {
				ImGui::Text("%s", item->name.c_str());

				if (item->isWeapon) {
					ImGui::Text("Damage %d-%d", item->weapon.minDamage, item->weapon.maxDamage);
					ImGui::Text("%g shots/s", item->weapon.shotPerSecond);
					ImGui::Text("range %g", item->weapon.rangeTiles);
				}

				for (int i = 0; i < Skill_Count; i++) {
					if (item->bonus[i] != 0) {
						ImGui::Text("%+d %s", item->bonus[i], StatLabels[i]);
					}
				}
				if (!item->ability.empty()) {
					ImGui::Text("Ability: %s", item->ability.c_str());
				}
			}
			ImGui::EndTooltip();
		}

		const Controls& controls;
		bool showEquipment = false;
		bool showBackpack = false;
		bool showCharacter = false;
		const InventoryUISettings& inventoryUI;
		const ItemDatabase& itemDatabase;
		AssetStore& assetStore;


		std::string inventoryMessage;        //last refusal from an inventory rule, e.g. "Backpack is full"
		Uint32 inventoryMessageTime = 0;     //when it was shown, in SDL ticks
		static constexpr const char* StatLabels[Skill_Count] = { "Attack", "Defense", "Wisdom", "Vitality", "Speed", "Dexterity" };
};

#endif // !RENDERGUISYSTEM_H

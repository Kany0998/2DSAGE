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
#include "../Components/LootBagComponent.h"
#include "../TileMap/TileMap.h"
#include "../TileMap/MovementType.h"
#include "../Game/Controls.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Events/LootDroppedEvent.h"
#include "../Events/ItemDroppedEvent.h"
#include "../Events/UISoundEvent.h"
#include "../Inventory/InventoryUISettings.h"
#include "../Inventory/ItemDatabase.h"
#include "../Inventory/InventoryRules.h"
#include "../Loot/LootDatabase.h"
#include "../Utils/EntityGeometry.h"
#include "../AssetStore/AssetStore.h"
#include "../Audio/AudioSettings.h"
#include <algorithm>
#include <cmath>
#include <SDL.h>


class RenderGUISystem
{
	public:
		RenderGUISystem(const Controls& controls, const InventoryUISettings& inventoryUI,
			const ItemDatabase& itemDatabase,const LootDatabase& lootDatabase, AudioSettings& audioSettings, std::unique_ptr<EventBus>& eventBus, AssetStore& assetStore) : controls(controls),
			inventoryUI(inventoryUI), itemDatabase(itemDatabase), lootDatabase(lootDatabase), audioSettings(audioSettings), eventBus(eventBus), assetStore(assetStore) { }

		void SubscribeToEvents()
		{
			eventBus->SubscribeToEvent<KeyPressedEvent>(this, &RenderGUISystem::OnKeyPressed);
			eventBus->SubscribeToEvent<LootDroppedEvent>(this, &RenderGUISystem::OnLootDropped);
		}

		void OnKeyPressed(KeyPressedEvent& event){
			if (event.isRepeat) return;                          //holding a key must not flicker the window
			if (ImGui::GetIO().WantCaptureKeyboard) return;      //typing into an ImGui field is not a toggle

			if (event.symbol == controls.equipment) showEquipment = !showEquipment;
			if (event.symbol == controls.backpack)  showBackpack = !showBackpack;
			if (event.symbol == controls.character) showCharacter = !showCharacter;
			if (event.symbol == controls.options)   showOptions = !showOptions;
		}

		void OnLootDropped(LootDroppedEvent& event) {
			
			//Do not use banner when item is dropped by player
			if (!event.fromKill) {
				return;
			}
			const BagTier* tier = lootDatabase.FindBag(event.tierId);
			if (tier == nullptr || tier->announceImage.empty()) {
				return;
			}


			bannerImage = tier->announceImage;
			bannerMs = tier->announceMs;
			bannerScale = tier->announceScale;
			bannerTime = SDL_GetTicks();
		}

		void Update(Registry& registry, const SDL_Rect& camera, bool isDebug, const TileMap& tilemap)
		{
			//Render GUI elements here
			ImGui::NewFrame();

			if (!bannerImage.empty()) {
				const Uint32 age = SDL_GetTicks() - bannerTime;
				if (age < static_cast<Uint32>(bannerMs)) {

					SDL_Texture *texture = assetStore.GetTexture(bannerImage);
					if (texture != nullptr) {
						int width = 0;
						int height = 0;
						SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

						const float drawWidth = static_cast<float>(width * bannerScale);
						const float drawHeight = static_cast<float>(height * bannerScale);

						const float timeLeft = static_cast<float>(bannerMs - static_cast<int>(age));
						const float fade = timeLeft < 1000.0f ? timeLeft / 1000.0f : 1.0f; //1 = fully visible, 0 = gone
						const int alpha = static_cast<int>(255.0f * fade);

						const ImVec2 min(16.0f, 16.0f);
						const ImVec2 max(min.x + drawWidth, min.y + drawHeight);
						ImGui::GetForegroundDrawList()->AddImage(texture, min, max, ImVec2(0, 0), ImVec2(1, 1),
							IM_COL32(255, 255, 255, alpha));
					}
				}
			}


			ImGuiIO& io = ImGui::GetIO();
			
			if (!io.WantCaptureMouse) {
				const glm::vec2 mouseWorld(io.MousePos.x + camera.x, io.MousePos.y + camera.y);
				Entity hoveredBag;
				if (FindBagAt(registry, mouseWorld, hoveredBag)) {
					const auto& transform = hoveredBag.GetComponent<TransformComponent>();
					const auto& sprite = hoveredBag.GetComponent<SpriteComponent>();

					const ImVec2 min(transform.position.x - camera.x,transform.position.y - camera.y);
					const ImVec2 max(min.x + sprite.width * transform.scale.x,min.y + sprite.height * transform.scale.y);
					ImGui::GetBackgroundDrawList()->AddRect(min , max, IM_COL32(255, 255, 255, 200), 0.0f, 0, 2.0f);

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						if (registry.HasEntityWithTag("player")) {

							Entity player = registry.GetEntityByTag("player");
							const glm::vec2 playerCentre = EntityCenter(player.GetComponent<TransformComponent>(),player.GetComponent<SpriteComponent>());
							
							const glm::vec2 bagCentre = EntityCenter(transform, sprite);
							const float distance = glm::length(playerCentre - bagCentre);
							const float range = lootDatabase.PickupRangeTiles() * static_cast<float>(tilemap.TileWorldSize());

							if (distance <= range) {

								//a sound only when this bag wasn't already open
								const bool alreadyOpen = hasOpenBag && openBag == hoveredBag;   //checked BEFORE overwriting openBag

								openBag = hoveredBag;
								hasOpenBag = true;

								if (!alreadyOpen) {
									eventBus->EmitEvent<UISoundEvent>("open_bag");
								}
							}
							else {
								ShowCursorMessage("Too far away");
							}
						}
					}
				}
			}

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
										else {
											eventBus->EmitEvent<UISoundEvent>("unequip");
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

								DrawInventoryMessage();

								ImGui::Separator();
								for (int i = 0; i < Skill_Count; i++) {
									StatText(static_cast<SkillStat>(i), attributes);
								}
							}
						}
						ImGui::End();
					}
					if (showBackpack) {
						
						if (player.HasComponent<BackpackComponent>() &&
							player.HasComponent<EquipmentComponent>() &&
							player.HasComponent<AttributesComponent>()) {

							auto& backpack = player.GetComponent<BackpackComponent>();
							auto& equipment = player.GetComponent<EquipmentComponent>();
							auto& attributes = player.GetComponent<AttributesComponent>();
							std::string fillText;

							if (backpack.capacity <= 0) {
								fillText = std::to_string(backpack.Count()) + " items";
							}

							else {
								fillText = std::to_string(backpack.Count()) + " / " + std::to_string(backpack.capacity);
							}

							const std::string title = "Backpack (" + fillText + ")###Backpack";

							if (ImGui::Begin(title.c_str(), &showBackpack, ImGuiWindowFlags_AlwaysAutoResize)) {
								
								const int columns = inventoryUI.backpackColumns;
								const int itemCount = static_cast<int>(backpack.items.size());

								//with a limit every square shows; unlimited shows the items plus a free row
								const int slotCount = backpack.capacity > 0 ? backpack.capacity : (itemCount / columns + 1) * columns;

								for (int i = 0; i < slotCount; i++) {
									if (i % columns != 0) {
										ImGui::SameLine();
									}

									const ItemDefinition* item = (i < static_cast<int>(backpack.items.size()) && !backpack.items[i].empty()) ? itemDatabase.Find(backpack.items[i]) : nullptr;

									ImGui::PushID(i);

									//the square being dragged out shows its item darker, so you can see where it came from
									const ImGuiPayload* dragged = ImGui::GetDragDropPayload();
									const bool isDragged = dragged != nullptr && dragged->IsDataType("BACKPACK_ITEM") && *static_cast<const int*>(dragged->Data) == i;

									DrawItemIcon(item, isDragged ? 100 : 255);
									const ImVec2 slotMin = ImGui::GetItemRectMin();      //topleft of the slot button just drawn

									//dropped a dragged backpack item onto this square: move or swap
									if (ImGui::BeginDragDropTarget()) {
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BACKPACK_ITEM", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
											const int from = *static_cast<const int*>(payload->Data);		//square the item was dragged from
											if (backpack.MoveItem(from, i)) {
												eventBus->EmitEvent<UISoundEvent>("move_item");
											}
										}
										ImGui::EndDragDropTarget();
									}


									if (item != nullptr && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
										std::string message;
										if (!InventoryRules::Equip(i, backpack, equipment, attributes, itemDatabase, message)) {
											ShowInventoryMessage(message);
										}
										else {
											eventBus->EmitEvent<UISoundEvent>("equip");
										}
									}

									if (item != nullptr && ImGui::IsItemHovered() && dragged == nullptr) {
										ImGui::BeginTooltip();
										DrawItemDetails(*item);
										ImGui::TextDisabled("Right-click to equip (%s)", item->tag.c_str());
										ImGui::EndTooltip();
									}

									//hold left mouse on an item and move: start dragging it; the payload is this square's index
									if(item != nullptr && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)){
										ImGui::SetDragDropPayload("BACKPACK_ITEM", &i, sizeof(int));
										//keep the point you grabbed under the cursor
										const ImGuiIO& io = ImGui::GetIO();
										const ImVec2 grab(io.MouseClickedPos[0].x - slotMin.x, io.MouseClickedPos[0].y - slotMin.y);
										const ImVec2 corner(io.MousePos.x - grab.x, io.MousePos.y - grab.y);
										DrawItemPicture(ImGui::GetForegroundDrawList(), corner, item);
										ImGui::EndDragDropSource();
									}

									

									ImGui::PopID();
								}
								DrawInventoryMessage();
							}
							ImGui::End();
						}
						
					}

					

					if (hasOpenBag) {
						if (!openBag.IsAlive() || !openBag.HasComponent<LootBagComponent>()) {
							hasOpenBag = false;
						}

						else {
							const glm::vec2 playerCenter = EntityCenter(player.GetComponent<TransformComponent>(), player.GetComponent<SpriteComponent>());
							const glm::vec2 bagCenter = EntityCenter(openBag.GetComponent<TransformComponent>(), openBag.GetComponent<SpriteComponent>());
							const float range = lootDatabase.PickupRangeTiles() * static_cast<float>(tilemap.TileWorldSize());

							if (glm::length(playerCenter - bagCenter) > range) {
								hasOpenBag = false;
							}
						}
					}

					if (hasOpenBag) {
						auto& bag = openBag.GetComponent<LootBagComponent>();
						const std::string title = "Loot: " + bag.tierId + "###LootBag";
						if (ImGui::Begin(title.c_str(), &hasOpenBag, ImGuiWindowFlags_AlwaysAutoResize)) {
							
							const int columns = inventoryUI.backpackColumns;
							if (player.HasComponent<BackpackComponent>()) {
								auto& backpack = player.GetComponent<BackpackComponent>();
								for (int i = 0; i < static_cast<int>(bag.items.size()); i++) {
									if (i % columns != 0) {
										ImGui::SameLine();
									}

									const ItemDefinition* item = itemDatabase.Find(bag.items[i]);
									ImGui::PushID(i);
									DrawItemIcon(item);
									if (item != nullptr && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
										const bool taken = backpack.Add(bag.items[i]);

										if (taken) {
											bag.items.erase(bag.items.begin() + i);
											ImGui::PopID();
											eventBus->EmitEvent<UISoundEvent>("take_item");
											break;
										}

										else {
											ShowInventoryMessage("Backpack is full");
										}
									}
									ImGui::PopID();
									if (item != nullptr && ImGui::IsItemHovered()) {
										ImGui::BeginTooltip();
										DrawItemDetails(*item);
										ImGui::TextDisabled("Right-click to take");
										ImGui::EndTooltip();
									}
								}
								DrawInventoryMessage();
							}
							
						}
						ImGui::End();
					}

					//a dragged backpack item released over the map (not over any window): drop it at the player's feet
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					if (payload != nullptr && payload->IsDataType("BACKPACK_ITEM")
						&& ImGui::IsMouseReleased(ImGuiMouseButton_Left)
						&& !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
						&& player.HasComponent<BackpackComponent>()) {

						auto& backpack = player.GetComponent<BackpackComponent>();
						const int index = *static_cast<const int*>(payload->Data);

						//the square must still exist and hold an item
						if (index >= 0 && index < static_cast<int>(backpack.items.size()) && !backpack.items[index].empty()) {
							const std::string itemId = backpack.items[index];		//copy before emptying the square
							backpack.items[index] = "";								//free square
							const glm::vec2 playerCenter = EntityCenter(player.GetComponent<TransformComponent>(), player.GetComponent<SpriteComponent>());
							eventBus->EmitEvent<ItemDroppedEvent>(playerCenter, itemId);
						}
					}



					if (!cursorMessage.empty() && SDL_GetTicks() - cursorMessageTime < 3000) {
						ImGui::GetForegroundDrawList()->AddText(ImVec2(io.MousePos.x + 16, io.MousePos.y),
							IM_COL32(255, 100, 100, 255), cursorMessage.c_str());
					}

				}
			}
			if (showOptions) {
				if (ImGui::Begin("Options", &showOptions, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::SliderFloat("Master", &audioSettings.master, 0.0f, 1.0f);
					ImGui::SliderFloat("Ambience", &audioSettings.ambience, 0.0f, 1.0f);
					ImGui::SliderFloat("Music", &audioSettings.music, 0.0f, 1.0f);
					ImGui::SliderFloat("Effects", &audioSettings.effects, 0.0f, 1.0f);
				}
				ImGui::End();
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

		//Draws a slot's frame and the item's icon with its top-left at corner, on any draw list - no widget, no ID.
		//Used by DrawItemIcon (in a window) and by the drag preview (on the foreground). brightness < 255 darkens the icon.
		void DrawItemPicture(ImDrawList* drawList, ImVec2 corner, const ItemDefinition* item, Uint8 brightness = 255) {
			
			//Size of one slot in pixels
			const float slot = inventoryUI.SlotPixels();

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
					drawList->AddImage(texture, iconMin, iconMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(brightness, brightness, brightness, 255));
				}
			}
		}


		//Draws one inventory slot at the cursor: a frame, then the item's icon fitted inside it.
		//Always takes exactly one slot of space, so rows and grids line up; item may be nullptr (empty slot).
		void DrawItemIcon(const ItemDefinition* item, Uint8 brightness = 255){
			
			//Size of one slot in pixels
			const float slot = inventoryUI.SlotPixels();
			DrawItemPicture(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), item, brightness);
			//an invisible button: reserves the space and gives the slot an ID, so it can be dragged and doesn't drag the window.
			ImGui::InvisibleButton("##slot", ImVec2(slot, slot));
		}

		//Shows a refusal from Equip/Unequip in the inventory windows for a few seconds
		void ShowInventoryMessage(const std::string& message){
			inventoryMessage = message;
			inventoryMessageTime = SDL_GetTicks();
		}

		//Shows a refusal from pickup near cursor for a few seconds
		void ShowCursorMessage(const std::string& message) {
			cursorMessage = message;
			cursorMessageTime = SDL_GetTicks();
		}

		//The last refusal from an inventory rule, shown for 3 seconds
		void DrawInventoryMessage() {
			if (!inventoryMessage.empty() && SDL_GetTicks() - inventoryMessageTime < 3000) {
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", inventoryMessage.c_str());
			}
		}

		//Hover tooltip for an equipment slot: the slot's name, then the item's details, or that it is empty
		void DrawSlotTooltip(const EquipmentSlot& slot, const ItemDefinition* item) {
			ImGui::BeginTooltip();
			ImGui::TextDisabled("%s", slot.name.c_str());	//slot name, greyed out on top

			if (item == nullptr) {
				ImGui::Text(slot.accepts == "special_ability" ? "No special ability" : "Empty");
			}

			else {
				DrawItemDetails(*item);
			}
			ImGui::EndTooltip();
		}

		//An item's details: name, weapon numbers, bonuses, ability. Used inside both tooltips.
		void DrawItemDetails(const ItemDefinition& item) {
			ImGui::Text("%s", item.name.c_str());

			if (item.isWeapon) {
				ImGui::Text("Damage %d-%d", item.weapon.minDamage, item.weapon.maxDamage);
				ImGui::Text("%g shots/s", item.weapon.shotPerSecond);
				ImGui::Text("range %g tiles", item.weapon.rangeTiles);
			}
			for (int i = 0; i < Skill_Count; i++) {
				if (item.bonus[i] != 0) {
					ImGui::Text("%+d %s", item.bonus[i], StatLabels[i]);
				}
			}
			if (!item.ability.empty()) {
				ImGui::Text("Ability: %s", item.ability.c_str());
			}
		}

		//Find bag for hovering
		bool FindBagAt(Registry& registry, glm::vec2 worldPoint, Entity& outBag) {

			bool found = false;        //has any bag contained the point yet?
			Uint32 newest = 0;         //spawn time of the best bag so far
			for (auto rawEntity : registry.Raw().view<TransformComponent, SpriteComponent, LootBagComponent>()) {
				
				Entity bag(rawEntity, &registry);
				const auto& bagTransform = bag.GetComponent<TransformComponent>();
				const auto& bagSprite = bag.GetComponent<SpriteComponent>();
				const auto& bagLoot = bag.GetComponent<LootBagComponent>();

				const glm::vec2 topLeft = bagTransform.position;
				const double width = bagSprite.width * bagTransform.scale.x;
				const double height = bagSprite.height * bagTransform.scale.y;

				if (!(worldPoint.x >= topLeft.x && worldPoint.x < topLeft.x + width && worldPoint.y >= topLeft.y && worldPoint.y < topLeft.y + height)) {
					continue;
				}

				if (!found || bagLoot.spawnTime > newest) {
					outBag = bag;
					newest = bagLoot.spawnTime;
					found = true;
				}
			}
			return found;
		}

		const Controls& controls;
		bool showEquipment = false;
		bool showBackpack = false;
		bool showCharacter = false;
		bool showOptions = false;
		const InventoryUISettings& inventoryUI;
		const ItemDatabase& itemDatabase;
		const LootDatabase& lootDatabase;
		AudioSettings& audioSettings;
		std::unique_ptr<EventBus>& eventBus;
		AssetStore& assetStore;


		Entity openBag;						 //the loot bag whose window is open
		bool hasOpenBag = false;			 //whether a bag window is open at all
		std::string inventoryMessage;        //last refusal from an inventory rule, e.g. "Backpack is full"
		std::string cursorMessage;          //world feedback shown next to the mouse, e.g. "Too far away"
		Uint32 cursorMessageTime = 0;
		Uint32 inventoryMessageTime = 0;     //when it was shown, in SDL ticks
		static constexpr const char* StatLabels[Skill_Count] = { "Attack", "Defense", "Wisdom", "Vitality", "Speed", "Dexterity" };

		std::string bannerImage;		//Texture for announce on legendary drops
		Uint32 bannerTime = 0;			//when it starts
		int bannerMs = 0;				//how long it lasts
		int bannerScale = 1;
};

#endif // !RENDERGUISYSTEM_H

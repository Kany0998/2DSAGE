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

class RenderGUISystem
{
	public:
		RenderGUISystem() = default;

		void Update(Registry& registry, const SDL_Rect& camera, bool isDebug)
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
					ImGui::Text("Map coordinates(x=%.1f, y=%.1f",
						ImGui::GetIO().MousePos.x + camera.x,
						ImGui::GetIO().MousePos.y + camera.y);
				}
				ImGui::End();
			}

			if (registry.HasEntityWithTag("player")) {
				Entity player = registry.GetEntityByTag("player");

				if (player.IsAlive() && player.HasComponent<ProgressionComponent>() && player.HasComponent<AttributesComponent>()) {

					//both non-const: spending a point writes to each of them
					auto& progression = player.GetComponent<ProgressionComponent>();
					auto& attributes = player.GetComponent<AttributesComponent>();

					ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_FirstUseEver);

					if (ImGui::Begin("Character")) {
						ImGui::Text("Level: %d", progression.currentLevel);
						ImGui::Text("%d / %d exp", progression.currentExperience, progression.experienceToNextLevel);
						ImGui::Text("Skill points: %d", progression.unspentSkillPoints);

						ImGui::Separator();

						// The stat order is declared once, here, and reused by both the rows
						// and the reset below. Two hand-written lists would only have to
						// agree with each other, and would silently refund the wrong stat
						// the day someone reordered one of them.
						int* stats[Skill_Count] = {
							&attributes.attackPower,
							&attributes.defensePower,
							&attributes.wisdomPower,
							&attributes.vitalityPower,
							&attributes.speedPower,
							&attributes.dexterityPower
						};
						const char* labels[Skill_Count] = {
							"Attack", "Defense", "Wisdom", "Vitality", "Speed", "Dexterity"
						};

						for (int i = 0; i < Skill_Count; i++)
						{
							StatRow(labels[i], *stats[i], progression.spentPoints[i], progression);
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
								*stats[i] -= progression.spentPoints[i];
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
			}

			ImGui::Render();
			ImGuiSDL::Render(ImGui::GetDrawData());
		}

	private:
		// One row of the character sheet. Takes the stat by reference so a single
		// helper serves all six - the row doesn't need to know which stat it edits,
		// only where it lives. When per-class caps arrive, the spend rule goes here
		// rather than into six separate button conditions.
		void StatRow(const char* label, int& stat, int& spent, ProgressionComponent& progression)
		{
			// All six buttons are labelled "+", and ImGui derives widget identity from
			// the label - without a unique id pushed around each row they would all be
			// the same widget. The stat names are distinct, so they serve as the id.
			ImGui::PushID(label);

			const bool canSpend = progression.unspentSkillPoints > 0;

			// ImGui 1.79 has no BeginDisabled(), so the row is dimmed by hand
			if (!canSpend) { ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f); }

			ImGui::Text("%-10s %6d (+%d)", label, stat, spent);
			ImGui::SameLine();

			// Button() is called unconditionally and the result filtered afterwards.
			// Testing canSpend first would short-circuit the call, and the buttons
			// would vanish rather than grey out whenever the pool hit zero.
			if (ImGui::Button("+") && canSpend)
			{
				progression.unspentSkillPoints--;
				stat++;
				spent++;
			}

			if (!canSpend) { ImGui::PopStyleVar(); }

			ImGui::PopID();
		}
};

#endif // !RENDERGUISYSTEM_H

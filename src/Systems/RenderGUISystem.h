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

class RenderGUISystem
{
	public:
		RenderGUISystem() = default;

		void Update(Registry& registry, const SDL_Rect& camera)
		{
			//Render GUI elements here
			ImGui::NewFrame();

			
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
				const char* sprites[] = { "tank-texture", "truck-texture", "chopper-texture"};
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
					ImGui::SliderInt("%", &health, 0,100);
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
					positionX = positionY  = rotate = projectileAngle = 0;
					scaleX = scaleY = 1;
					projectileRate = projectileDuration = projectileDamage = 10;
					projectileSpeed = 100;
					health = 100;
				}
			}
			ImGui::End();

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav;
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0,0));
			ImGui::SetNextWindowBgAlpha(0.5f);
			if (ImGui::Begin("Map Coordinates", NULL, windowFlags))
			{
				ImGui::Text("Map coordinates(x=%.1f, y=%.1f",
					ImGui::GetIO().MousePos.x + camera.x,
					ImGui::GetIO().MousePos.y + camera.y);
			}
			ImGui::End();

			ImGui::Render();
			ImGuiSDL::Render(ImGui::GetDrawData());
		}
		
};

#endif // !RENDERGUISYSTEM_H

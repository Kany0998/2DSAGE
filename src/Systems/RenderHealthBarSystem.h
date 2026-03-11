#ifndef RENDERHEALTHBARSYSTEM_H
#define RENDERHEALTHBARSYSTEM_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"

#include <SDL.h>

class RenderHealthBarSystem : public System
{
	public:
		RenderHealthBarSystem()
		{
			RequireComponent<HealthComponent>();
			RequireComponent<TransformComponent>();
			RequireComponent<SpriteComponent>();
		}

		void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera)
		{
			for (auto entity : GetSystemEntities())
			{
				const auto transform = entity.GetComponent<TransformComponent>();
				const auto sprite = entity.GetComponent<SpriteComponent>();
				const auto health = entity.GetComponent<HealthComponent>();

				//Draw healthbar with correct color based on health percentagee
				SDL_Color healthBarColor = {255,255,255};
				//always draw backgroung of health bar in gray color
				SDL_Color healthBarBackGroundColor = { 128, 128, 128,};

				if (health.healthPercentage >= 0 && health.healthPercentage < 30)
				{
					healthBarColor = { 255,0,0 };//red
				}
				if (health.healthPercentage >= 30 && health.healthPercentage < 70)
				{
					healthBarColor = { 255,255,0 };//yeallow
				}

				if (health.healthPercentage >= 70 && health.healthPercentage <=100)
				{
					healthBarColor = { 0,255,0 };//green
				}

				//postions of health bar
				int healthBarWidth = sprite.width * transform.scale.x - 5;
				int healthBarHeight = 5;
				double healthBarX = transform.position.x - camera.x + 2.5;
				double healthBarY = (transform.position.y + sprite.height * transform.scale.y) - camera.y + 2 ;

				SDL_Rect healthBarRectangle = {
					static_cast<int>(healthBarX),
					static_cast<int>(healthBarY),
					static_cast<int>(healthBarWidth * (health.healthPercentage / 100.0)),
					static_cast<int>(healthBarHeight)
				};
				SDL_Rect healthBarBackRectangle = {
					static_cast<int>(healthBarX),
					static_cast<int>(healthBarY),
					static_cast<int>(healthBarWidth),
					static_cast<int>(healthBarHeight)
				};
				SDL_SetRenderDrawColor(renderer, healthBarBackGroundColor.r, healthBarBackGroundColor.g, healthBarBackGroundColor.b, 255);
				SDL_RenderFillRect(renderer, &healthBarBackRectangle);

				SDL_SetRenderDrawColor(renderer, healthBarColor.r, healthBarColor.g, healthBarColor.b, 255);
				SDL_RenderFillRect(renderer, &healthBarRectangle);

			}
		}
};


#endif // !RENDERHEALTHBARSYSTEM_H

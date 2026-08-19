#ifndef RENDERHEALTHBARSYSTEM_H
#define RENDERHEALTHBARSYSTEM_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "./BarText.h"

#include <SDL.h>
#include <string>

class RenderHealthBarSystem
{
	public:
		RenderHealthBarSystem() = default;

		void Update(Registry& registry, SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera)
		{
			for (auto rawEntity : registry.Raw().view<HealthComponent, TransformComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				const auto transform = entity.GetComponent<TransformComponent>();
				const auto sprite = entity.GetComponent<SpriteComponent>();
				const auto health = entity.GetComponent<HealthComponent>();

				//Draw healthbar with correct color based on health percentage
				//(healthPoints isn't necessarily 0-100 - normalize against maxHealthPoints first)
				double healthPercent = (health.healthPoints / (double)health.maxHealthPoints) * 100.0;
				//overkill damage can push healthPoints negative for the one frame before
				//the entity is actually destroyed (Kill() is deferred) - clamp so the bar
				//can't compute a negative width and render growing backwards
				healthPercent = healthPercent < 0.0 ? 0.0 : (healthPercent > 100.0 ? 100.0 : healthPercent);

				SDL_Color healthBarColor = {255,255,255};
				//always draw backgroung of health bar in gray color
				SDL_Color healthBarBackGroundColor = { 128, 128, 128,};

				if (healthPercent >= 0 && healthPercent < 30)
				{
					healthBarColor = { 255,0,0 };//red
				}
				if (healthPercent >= 30 && healthPercent < 70)
				{
					healthBarColor = { 255,255,0 };//yeallow
				}

				if (healthPercent >= 70 && healthPercent <=100)
				{
					healthBarColor = { 0,255,0 };//green
				}

				const std::string healthText = std::to_string(health.healthPoints) + "/" + std::to_string(health.maxHealthPoints);

				//Fixed size rather than sprite-relative - a 27px strip scaled off a 32px
				//sprite can't hold a caption - and it only grows when the caption would
				//otherwise be clipped, so bars stay uniform for ordinary numbers and
				//widen for something like "8593/10000".
				int textWidth = 0;
				int textHeight = 0;
				BarText::Measure(assetStore, healthText, textWidth, textHeight);

				int healthBarWidth = BarWidth;
				if (textWidth + 2 * TextPadding > healthBarWidth)
				{
					healthBarWidth = textWidth + 2 * TextPadding;
				}

				int healthBarHeight = BarHeight;

				//centred under the sprite, so a widened bar grows evenly to both sides
				//instead of drifting off to the right
				const double spriteCenterX = transform.position.x + (sprite.width * transform.scale.x) / 2.0;
				double healthBarX = spriteCenterX - camera.x - healthBarWidth / 2.0;
				double healthBarY = (transform.position.y + sprite.height * transform.scale.y) - camera.y + 2 ;

				SDL_Rect healthBarRectangle = {
					static_cast<int>(healthBarX),
					static_cast<int>(healthBarY),
					static_cast<int>(healthBarWidth * (healthPercent / 100.0)),
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

				BarText::DrawCentered(renderer, assetStore, healthText, healthBarBackRectangle);
			}
		}

	private:
		static constexpr int BarWidth = 70;		//the size every bar keeps unless its caption doesn't fit
		static constexpr int BarHeight = 12;
		static constexpr int TextPadding = 3;	//breathing room either side of the caption
};


#endif // !RENDERHEALTHBARSYSTEM_H

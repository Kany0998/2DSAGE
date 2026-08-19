#ifndef RENDERMANABARSYSTEM_H
#define RENDERMANABARSYSTEM_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/ManaComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "./BarText.h"

#include <SDL.h>
#include <string>

class RenderManaBarSystem
{
public:
	RenderManaBarSystem() = default;

	void Update(Registry& registry, SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera)
	{
		for (auto rawEntity : registry.Raw().view<ManaComponent, TransformComponent, SpriteComponent>())
		{
			Entity entity(rawEntity, &registry);

			const auto transform = entity.GetComponent<TransformComponent>();
			const auto sprite = entity.GetComponent<SpriteComponent>();
			const auto mana = entity.GetComponent<ManaComponent>();

			//Draw mana bar with correct color based on mana percentage
			//(manaPoints isn't necessarily 0-100 - normalize against maxManaPoints first)
			double manaPercent = (mana.manaPoints / (double)mana.maxManaPoints) * 100.0;
			manaPercent = manaPercent < 0.0 ? 0.0 : (manaPercent > 100.0 ? 100.0 : manaPercent);

			SDL_Color manaBarColor = { 255,255,255 };
			//always draw background of mana bar in gray color
			SDL_Color manaBarBackGroundColor = { 128, 128, 128, };

			manaBarColor = {0,137,255};//blue


			const std::string manaText = std::to_string(mana.manaPoints) + "/" + std::to_string(mana.maxManaPoints);

			//same sizing rule as the health bar: a fixed width that only grows when the
			//caption would be clipped
			int textWidth = 0;
			int textHeight = 0;
			BarText::Measure(assetStore, manaText, textWidth, textHeight);

			int manaBarWidth = BarWidth;
			if (textWidth + 2 * TextPadding > manaBarWidth)
			{
				manaBarWidth = textWidth + 2 * TextPadding;
			}

			int manaBarHeight = BarHeight;

			//centred under the sprite, and directly under the health bar - which is
			//BarHeight tall and starts 2px below the sprite
			const double spriteCenterX = transform.position.x + (sprite.width * transform.scale.x) / 2.0;
			double manaBarX = spriteCenterX - camera.x - manaBarWidth / 2.0;
			double manaBarY = (transform.position.y + sprite.height * transform.scale.y) - camera.y + 2 + BarHeight + 2;

			SDL_Rect manaBarRectangle = {
				static_cast<int>(manaBarX),
				static_cast<int>(manaBarY),
				static_cast<int>(manaBarWidth * (manaPercent / 100.0)),
				static_cast<int>(manaBarHeight)
			};
			SDL_Rect manaBarBackRectangle = {
				static_cast<int>(manaBarX),
				static_cast<int>(manaBarY),
				static_cast<int>(manaBarWidth),
				static_cast<int>(manaBarHeight)
			};
			SDL_SetRenderDrawColor(renderer, manaBarBackGroundColor.r, manaBarBackGroundColor.g, manaBarBackGroundColor.b, 255);
			SDL_RenderFillRect(renderer, &manaBarBackRectangle);

			SDL_SetRenderDrawColor(renderer, manaBarColor.r, manaBarColor.g, manaBarColor.b, 255);
			SDL_RenderFillRect(renderer, &manaBarRectangle);

			BarText::DrawCentered(renderer, assetStore, manaText, manaBarBackRectangle);
		}
	}

private:
	static constexpr int BarWidth = 70;		//matches RenderHealthBarSystem so the two stack evenly
	static constexpr int BarHeight = 12;
	static constexpr int TextPadding = 3;
};


#endif // RENDERMANABARSYSTEM_H

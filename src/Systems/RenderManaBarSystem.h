#ifndef RENDERMANABARSYSTEM_H
#define RENDERMANABARSYSTEM_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/ManaComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"

#include <SDL.h>

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

			manaBarColor = {0,137,255};//red
			

			//postions of mana bar
			int manaBarWidth = sprite.width * transform.scale.x - 5;
			int manaBarHeight = 5;
			double manaBarX = transform.position.x - camera.x + 2;
			double manaBarY = (transform.position.y + sprite.height * transform.scale.y) - camera.y + 7;

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

		}
	}
};


#endif // RENDERMANABARSYSTEM_H

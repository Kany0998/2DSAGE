#ifndef RENDEREXPERIENCEBARSYSTEM_H
#define RENDEREXPERIENCEBARSYSTEM_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/ProgressionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "./BarText.h"

#include <SDL.h>
#include <string>

// Draws a light green progress bar off the top-right corner of the entity's sprite,
// captioned "<level> lvl" against its left edge and "<current>/<needed>" in its
// centre. Same shape as RenderHealthBarSystem/RenderManaBarSystem.
class RenderExperienceBarSystem
{
	public:
		RenderExperienceBarSystem() = default;

		void Update(Registry& registry, SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera)
		{
			for (auto rawEntity : registry.Raw().view<ProgressionComponent, TransformComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				const auto transform = entity.GetComponent<TransformComponent>();
				const auto sprite = entity.GetComponent<SpriteComponent>();
				const auto progression = entity.GetComponent<ProgressionComponent>();

				//How far into the current level the entity is. ProgressionSystem::Update()
				//guarantees a non-zero threshold before anything renders, but a bar that
				//divides by it shouldn't depend on someone else's invariant.
				double experiencePercent = 0.0;
				if (progression.experienceToNextLevel > 0)
				{
					experiencePercent = (progression.currentExperience / (double)progression.experienceToNextLevel) * 100.0;
				}
				experiencePercent = experiencePercent < 0.0 ? 0.0 : (experiencePercent > 100.0 ? 100.0 : experiencePercent);

				const std::string levelText = std::to_string(progression.currentLevel) + " lvl";
				const std::string experienceText = std::to_string(progression.currentExperience) + "/" + std::to_string(progression.experienceToNextLevel);

				//Same sizing rule as the health/mana bars: a fixed width that only grows
				//when the captions would collide. Here that takes two measurements, since
				//the level caption is left-aligned and the total is centred - the centred
				//text must start after the level caption ends, which works out as
				//width >= centred + 2 * (padding + level + gap). High levels push the
				//experience numbers into the thousands, so this does get exercised.
				int levelWidth = 0;
				int experienceWidth = 0;
				int textHeight = 0;
				BarText::Measure(assetStore, levelText, levelWidth, textHeight);
				BarText::Measure(assetStore, experienceText, experienceWidth, textHeight);

				int barWidth = BarWidth;
				const int requiredWidth = experienceWidth + 2 * (TextPadding + levelWidth + Gap);
				if (requiredWidth > barWidth)
				{
					barWidth = requiredWidth;
				}

				//anchored on the sprite's top-right corner, converted to screen space the
				//same way the other bars do it
				const double barX = (transform.position.x + sprite.width * transform.scale.x) - camera.x + Gap;
				const double barY = transform.position.y - camera.y;

				SDL_Rect experienceBarBackRectangle = {
					static_cast<int>(barX),
					static_cast<int>(barY),
					static_cast<int>(barWidth),
					static_cast<int>(BarHeight)
				};
				SDL_Rect experienceBarRectangle = {
					static_cast<int>(barX),
					static_cast<int>(barY),
					static_cast<int>(barWidth * (experiencePercent / 100.0)),
					static_cast<int>(BarHeight)
				};

				SDL_SetRenderDrawColor(renderer, BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 255);
				SDL_RenderFillRect(renderer, &experienceBarBackRectangle);

				SDL_SetRenderDrawColor(renderer, BarColor.r, BarColor.g, BarColor.b, 255);
				SDL_RenderFillRect(renderer, &experienceBarRectangle);

				//both captions sit inside the bar: the level against the left edge, the
				//experience total in the middle. The bar is wide enough that they don't
				//collide until the numbers get very long.
				BarText::DrawLeftAligned(renderer, assetStore, levelText, experienceBarBackRectangle, TextPadding);
				BarText::DrawCentered(renderer, assetStore, experienceText, experienceBarBackRectangle);
			}
		}

	private:
		static constexpr int BarWidth = 110;	//wide enough to hold both captions side by side
		static constexpr int BarHeight = 12;
		static constexpr int Gap = 4;			//spacing between the sprite and the bar
		static constexpr int TextPadding = 3;	//inset of the level caption from the bar's left edge

		static constexpr SDL_Color BarColor = { 144, 238, 144, 255 };			//light green
		static constexpr SDL_Color BackgroundColor = { 128, 128, 128, 255 };	//same gray the health/mana bars use
};

#endif // !RENDEREXPERIENCEBARSYSTEM_H

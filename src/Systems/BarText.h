#ifndef BARTEXT_H
#define BARTEXT_H

#include "../AssetStore/AssetStore.h"
#include <SDL.h>
#include <memory>
#include <string>

// Shared text drawing for the health/mana/experience bars. Lives in one place
// because all three need the same font lookup, the same missing-font guard and the
// same alpha handling - three copies of that would drift.
namespace BarText
{
	static constexpr const char* FontAssetId = "charriot-font";

	// Alpha is spelled out on purpose: SDL_Color's three-value aggregate init leaves
	// a = 0, and blended text at zero alpha renders completely invisible.
	static constexpr SDL_Color TextColor = { 0, 0, 0, 255 };	//black

	// Measures text without rendering it, so a bar can size itself to its caption
	// before it is drawn. Reports 0x0 when the font is missing, which leaves callers
	// on their fixed size rather than collapsing the bar to nothing.
	inline void Measure(std::unique_ptr<AssetStore>& assetStore, const std::string& text, int& width, int& height)
	{
		width = 0;
		height = 0;

		TTF_Font* font = assetStore->GetFont(FontAssetId);
		if (font == nullptr)
		{
			return;
		}

		TTF_SizeText(font, text.c_str(), &width, &height);
	}

	// Returns nullptr when the font was never loaded - AssetStore::GetFont() is a map
	// lookup that yields nullptr for an unknown id, and TTF_RenderText_Blended would
	// dereference it. Callers draw their bar without a caption rather than crash.
	// The caller owns the returned texture.
	inline SDL_Texture* CreateTexture(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const std::string& text)
	{
		TTF_Font* font = assetStore->GetFont(FontAssetId);
		if (font == nullptr)
		{
			return nullptr;
		}

		SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), TextColor);
		if (surface == nullptr)
		{
			return nullptr;
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		return texture;
	}

	// Draws text centred inside the given bar rectangle.
	inline void DrawCentered(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const std::string& text, const SDL_Rect& bar)
	{
		SDL_Texture* texture = CreateTexture(renderer, assetStore, text);
		if (texture == nullptr)
		{
			return;
		}

		int width = 0;
		int height = 0;
		SDL_QueryTexture(texture, NULL, NULL, &width, &height);

		SDL_Rect destination = {
			static_cast<int>(bar.x + (bar.w - width) / 2.0),
			static_cast<int>(bar.y + (bar.h - height) / 2.0),
			width,
			height
		};

		SDL_RenderCopy(renderer, texture, NULL, &destination);
		SDL_DestroyTexture(texture);
	}

	// Draws text against the left edge of the bar, vertically centred.
	inline void DrawLeftAligned(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const std::string& text, const SDL_Rect& bar, int padding)
	{
		SDL_Texture* texture = CreateTexture(renderer, assetStore, text);
		if (texture == nullptr)
		{
			return;
		}

		int width = 0;
		int height = 0;
		SDL_QueryTexture(texture, NULL, NULL, &width, &height);

		SDL_Rect destination = {
			bar.x + padding,
			static_cast<int>(bar.y + (bar.h - height) / 2.0),
			width,
			height
		};

		SDL_RenderCopy(renderer, texture, NULL, &destination);
		SDL_DestroyTexture(texture);
	}
}

#endif // !BARTEXT_H

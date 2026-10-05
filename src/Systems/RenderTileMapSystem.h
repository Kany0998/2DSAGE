#ifndef RENDERTILEMAPSYSTEM_H
#define RENDERTILEMAPSYSTEM_H

#include "../TileMap/TileMap.h"
#include "../AssetStore/AssetStore.h"
#include "../Logger/Logger.h"
#include <SDL.h>
#include <algorithm>

class RenderTileMapSystem {

	public:

		RenderTileMapSystem() = default;

		//Draws the map straight from the TileMap grid - only the tiles the camera can see
		void Update(SDL_Renderer* renderer, const std::unique_ptr<AssetStore>& assetStore,
			const SDL_Rect& camera, const TileMap& tileMap) {
			
			static bool warned = false;
			SDL_Texture* texture = assetStore->GetTexture(tileMap.TextureAssetId());

			if (texture == nullptr) {
				if (!warned) {
					Logger::Err("TileMap Render System Error: texture name might be wrongly typed and it give nullptr");
					warned = true;
				}
				return;
			}

			const int firstCol = std::max(0, tileMap.colAt(camera.x));
			const int lastCol = std::min(tileMap.Cols() - 1, tileMap.colAt(camera.x + camera.w));
			const int firstRow = std::max(0, tileMap.rowAt(camera.y));
			const int lastRow = std::min(tileMap.Rows() - 1, tileMap.rowAt(camera.y + camera.h));



			const int tileSize = tileMap.TileSize();
			const double tileWorldSize = tileMap.TileWorldSize();
			const int tilesPerRow = TilesPerRow(texture, tileSize);

			if (tilesPerRow <= 0) {
				return;
			}

			for (int row = firstRow; row <= lastRow; row++) {
				for (int col = firstCol; col <= lastCol; col++) {
					const int id = tileMap.tileAt(col, row);

					//Where this tile sits in the tileset image
					SDL_Rect srcRect = { 
						(id % tilesPerRow) * tileSize,
						(id / tilesPerRow) * tileSize,
						tileSize,
						tileSize 
					};

					//Where it goes on screen
					SDL_Rect dstRect = { 
						static_cast<int>(col * tileWorldSize - camera.x),
						static_cast<int>(row * tileWorldSize - camera.y),
						static_cast<int>(tileWorldSize),
						static_cast<int>(tileWorldSize)
					};

					SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
				}
			}
		}


	private:

		//How many tiles fit across the tileset image
		static int TilesPerRow(SDL_Texture* texture, int tileSize) {
			int textureWidth = 0;
			SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, nullptr);
			return tileSize > 0 ? textureWidth / tileSize : 0;
		}

};


#endif
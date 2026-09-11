#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../TileMap/TileMap.h"

#include <SDL.h>
#include <sol/sol.hpp>

#include <memory>

class TileMap;

class LevelLoader
{
	public:
		LevelLoader();
		~LevelLoader();

        void LoadLevel(
            sol::state& lua,
            const std::unique_ptr<Registry>& registry,
            const std::unique_ptr<AssetStore>& assetStore,
			TileMap& tileMap,
            SDL_Renderer* renderer,
            int levelNumber
        );
};

#endif
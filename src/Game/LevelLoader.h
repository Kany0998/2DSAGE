#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"

#include <SDL.h>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

#include <memory>

class LevelLoader
{
	public:
		LevelLoader();
		~LevelLoader();

        void LoadLevel(
            sol::state& lua,
            const std::unique_ptr<Registry>& legacyRegistry,
            entt::registry& enttRegistry,
            const std::unique_ptr<AssetStore>& assetStore,
            SDL_Renderer* renderer,
            int levelNumber
        );
};

#endif
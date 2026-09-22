#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components//SpriteComponent.h"
#include "../Components/AIComponent.h"
#include "../AssetStore/AssetStore.h"
#include <algorithm>
#include <SDL.h>


class RenderSystem
{
	public:
		RenderSystem() = default;

		void Update(Registry& registry, SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera)
		{
			//Create vector with sprite and transform component of all entites
			struct RenderableEntity
			{
				TransformComponent transformComponent;
				SpriteComponent spriteComponent;
				Uint8 alpha = 255;
			};

			std::vector<RenderableEntity> renderableEntities;

			for (auto rawEntity : registry.Raw().view<TransformComponent, SpriteComponent>())
			{
				Entity entity(rawEntity, &registry);

				RenderableEntity renderableEntity;
				renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();
				renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();

				if (entity.HasComponent<AIComponent>()) {

					const auto& enemyAI = entity.GetComponent<AIComponent>();
					if (enemyAI.state == AIState::Return) {
						renderableEntity.alpha = 128;
					}
				}

				//Bypass rendering entites if they are outside the camera view
				bool isEntityOutsideCameraView = (
					renderableEntity.transformComponent.position.x + (renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width)  < camera.x ||
					renderableEntity.transformComponent.position.x > camera.x + camera.w ||
					renderableEntity.transformComponent.position.y + (renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height) < camera.y ||
					renderableEntity.transformComponent.position.y > camera.y + camera.h
					);

				//cull sprites that are outside the camera view (and are not fixed)
				if (isEntityOutsideCameraView && !renderableEntity.spriteComponent.isFixed)
				{
					continue;
				}
				renderableEntities.emplace_back(renderableEntity);
			}
			//Sorting the vector by orderLayer
			std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
				return a.spriteComponent.layer < b.spriteComponent.layer;
				});


			//Loop all entites that the system is intested in
			for (auto entity : renderableEntities)
			{
				const auto transform = entity.transformComponent;
				const auto sprite = entity.spriteComponent;


				//Set source rectangle of out orginal sprite texure
				SDL_Rect srcRect = sprite.srcRect;

				// set destination rectangle with the x,y position to be rendered
				SDL_Rect dstRect = {
					static_cast<int>(transform.position.x - (sprite.isFixed ? 0 : camera.x)),
					static_cast<int>(transform.position.y - (sprite.isFixed ? 0 : camera.y)),
					static_cast<int>(sprite.width * transform.scale.x),
					static_cast<int>(sprite.height * transform.scale.y)
				};
				SDL_Point center = {
					dstRect.w / 2,
					dstRect.h / 2
				};
				SDL_Texture* texture = assetStore->GetTexture(sprite.assetId);
				SDL_SetTextureAlphaMod(texture, entity.alpha);

				SDL_RenderCopyEx(
					renderer,
					texture,
					&srcRect,
					&dstRect,
					transform.rotation,
					NULL,
					sprite.flip
				);

				SDL_SetTextureAlphaMod(texture, 255);
				//draw png texute based on spriteId
			}
		}
};

#endif


#ifndef RENDERCOLLISIONSYSTEM_H
#define RENDERCOLLISIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL.h>

class RenderCollisionSystem
{
public:
	RenderCollisionSystem() = default;

	void Update(Registry& registry, SDL_Renderer* renderer, const SDL_Rect& camera)
	{
		for (auto rawEntity: registry.Raw().view<TransformComponent, BoxColliderComponent>())
		{
			Entity entity(rawEntity, &registry);

			const auto transform = entity.GetComponent<TransformComponent>();
			const auto collider = entity.GetComponent<BoxColliderComponent>();

			SDL_Rect colliderRect = {
				static_cast<int>(transform.position.x + collider.offset.x - camera.x),
				static_cast<int>(transform.position.y + collider.offset.y - camera.y),
				static_cast<int>(collider.width * transform.scale.x),
				static_cast<int>(collider.height * transform.scale.y)
			};

			if (collider.isColliding)
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			}

			else
			{
				SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
			}
			SDL_RenderDrawRect(renderer, &colliderRect);
		}
	}
};

#endif
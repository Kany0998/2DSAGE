#ifndef RENDERCOLLSIONSYSTEM_H
#define RENDERCOLLISONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL.h>

class RenderCollisionSystem : public System
{
public:
	RenderCollisionSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<BoxColliderComponent>();
	}

	void Update(SDL_Renderer* renderer, const SDL_Rect& camera)
	{
		for (auto entity: GetSystemEntities())
		{
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
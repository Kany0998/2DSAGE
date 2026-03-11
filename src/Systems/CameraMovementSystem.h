#ifndef CAMERAMOVEMENTSYSTEM_H
#define CAMERAMOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/CameraHollderComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <SDL.h>

class CameraMovementSystem : public System
{
	public:
		CameraMovementSystem()
		{
			RequireComponent<CameraHollderComponent>();
			RequireComponent<TransformComponent>();
			RequireComponent<SpriteComponent>();
		}
		
		void Update(SDL_Rect& camera)
		{
            for (auto entity : GetSystemEntities())
            {
                auto transform = entity.GetComponent<TransformComponent>();
                auto sprite = entity.GetComponent<SpriteComponent>();

                //center of the spire
                int playerCenterX = transform.position.x + (sprite.width * transform.scale.x) / 2;
                int playerCenterY = transform.position.y + (sprite.height * transform.scale.y) / 2;

				//camera set on the center of the player
                camera.x = playerCenterX - (Game::windowWidth / 2);
                camera.y = playerCenterY - (Game::windowHeight / 2);

				//camera boundr for left and top edge
                camera.x = camera.x < 0 ? 0 : camera.x;
                camera.y = camera.y < 0 ? 0 : camera.y;

				//camera boundry for right and bottom edge
                int maxCameraX = Game::mapWidth - camera.w;
                int maxCameraY = Game::mapHeight - camera.h;

                camera.x = camera.x > maxCameraX ? maxCameraX : camera.x;
                camera.y = camera.y > maxCameraY ? maxCameraY : camera.y;
            }
		}
};

#endif // !CAMERAMOVEMENTSYSTEM_H

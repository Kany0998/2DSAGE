#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/RigidBodyComponent.h"
#include <SDL.h>
#include <glm/glm.hpp>

class AnimationSystem
{
	public:
		AnimationSystem() = default;

		void Update(Registry& registry)
		{
			for (auto rawEntity : registry.Raw().view<SpriteComponent, AnimationComponent>())
			{
				Entity entity(rawEntity, &registry);

				auto& animation = entity.GetComponent<AnimationComponent>();
				auto& sprite = entity.GetComponent<SpriteComponent>();

				if (animation.onlyWhenMoving && (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().velocity == glm::vec2(0.0, 0.0))) {
					animation.currentFrame = 0;
					animation.startTime = SDL_GetTicks();
					sprite.srcRect.x = 0;
					continue;
				}

				//how much time went from start of animation
				int elapsed = SDL_GetTicks() - animation.startTime;
				//change current frame
				int frame = (elapsed * animation.frameSpeedRate / 1000);
				if (animation.isLoop)
				{
					animation.currentFrame = frame % animation.numFrames;
				}
				else
				{
					if (frame >= animation.numFrames)
					{
						animation.currentFrame = animation.numFrames - 1;
					}
					else animation.currentFrame = frame;
				}

				sprite.srcRect.x = animation.currentFrame * sprite.width;
			}
		}
};




#endif
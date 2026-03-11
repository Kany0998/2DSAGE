#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include <SDL.h>

class AnimationSystem : public System
{
	public:
		AnimationSystem()
		{
			RequireComponent<SpriteComponent>();
			RequireComponent<AnimationComponent>();
		}

		void Update()
		{
			for (auto entity : GetSystemEntities())
			{
				auto& animation = entity.GetComponent<AnimationComponent>();
				auto& sprite = entity.GetComponent<SpriteComponent>();

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
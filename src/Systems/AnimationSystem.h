#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/ProjectileFiredEvent.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <cmath>
#include <algorithm>

class AnimationSystem
{
	public:
		AnimationSystem() = default;

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
			eventBus->SubscribeToEvent<ProjectileFiredEvent>(this, &AnimationSystem::OnProjectileFired);
		}

		void OnProjectileFired(ProjectileFiredEvent& event) {
			Entity shooter = event.shooter;

			if (!shooter.IsAlive()) {
				return;
			}

			if (!shooter.HasComponent<SpriteComponent>() || !shooter.HasComponent<AnimationComponent>()) {
				return;
			}

			auto& animation = shooter.GetComponent<AnimationComponent>();
			auto& sprite = shooter.GetComponent<SpriteComponent>();

			if (animation.attackAssetId.empty()) {
				return;
			}

			if (!animation.isAttacking) {
				animation.walkAssetId = sprite.assetId;
			}
			animation.isAttacking = true;
			animation.attackStartTime = SDL_GetTicks();
			sprite.assetId = animation.attackAssetId;

			// the pose lasts at most half the time to the next shot, so it shows on every shot
			// even at high dexterity; 30 ms keeps it visible for about two frames
			const int luaMs = animation.attackNumFrames * 1000 / animation.attackSpeedRate;
			const int shareMs = event.shotIntervalMs / 2;

			animation.attackDurationMs = std::max(30, std::min(luaMs, shareMs));
			animation.facingLockUntil = animation.attackStartTime + event.shotIntervalMs + facingGraceMs;

			glm::vec2 direction = event.direction;

			if (shooter.HasTag("player")) {
				if (std::abs(direction.x) >= std::abs(direction.y)) {
					animation.attackRow = direction.x > 0 ? 1 : 3;
				}
				
				else {
					animation.attackRow = direction.y > 0 ? 2 : 0;
				}
			}

			else {
				animation.attackFacesLeft = direction.x < 0;
			}

			sprite.srcRect.x = 0;
			sprite.srcRect.y = animation.attackRow * sprite.height;

			if (!shooter.HasTag("player")) {
				sprite.flip = animation.attackFacesLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			}
		}

		void Update(Registry& registry)
		{
			for (auto rawEntity : registry.Raw().view<SpriteComponent, AnimationComponent>())
			{
				Entity entity(rawEntity, &registry);

				auto& animation = entity.GetComponent<AnimationComponent>();
				auto& sprite = entity.GetComponent<SpriteComponent>();

				// flip while shooting, face the shot otherwise face the way you walk
				const bool locked = static_cast<int>(SDL_GetTicks()) < animation.facingLockUntil;
				bool faceLeft = false;

				if (locked) {
					faceLeft = animation.attackFacesLeft;
				}

				else if (entity.HasComponent<RigidBodyComponent>()) {
					faceLeft = entity.GetComponent<RigidBodyComponent>().velocity.x < 0;
				}

				sprite.flip = (faceLeft && !entity.HasTag("player")) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

				// before onlyWhenMoving: enemies usually stand still while they shoot
				if (animation.isAttacking) {
					Uint32 now = SDL_GetTicks();
					const int elapsed = static_cast<int>(now) - animation.attackStartTime;

					if (elapsed >= animation.attackDurationMs) {
						animation.isAttacking = false;
						sprite.assetId = animation.walkAssetId;
						animation.startTime = now;
					}

					else {
						const int frame = elapsed * animation.attackNumFrames / animation.attackDurationMs;
						sprite.srcRect.x = frame * sprite.width;
						sprite.srcRect.y = animation.attackRow * sprite.height;
						continue;
					}
				}

				// between shots the walk sheet uses the aim row (walk and attack sheets share the row order)
				if (entity.HasTag("player") && locked) {
					sprite.srcRect.y = animation.attackRow * sprite.height;
				}

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

	private:
		static constexpr int facingGraceMs = 100;   // extra time after the next expected shot
};




#endif
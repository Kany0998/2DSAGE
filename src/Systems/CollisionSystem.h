#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components//SpriteComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Utils/EntityGeometry.h"

class CollisionSystem{
	public:
		CollisionSystem() = default;

		int pairsTested = 0;
		int collisionsEvents = 0;

		void Update(Registry& registry, std::unique_ptr<EventBus>& eventBus){
			
			pairsTested = 0;
			collisionsEvents = 0;
			colliders.clear();

			// read once per entity
			for (auto rawEntity : registry.Raw().view<TransformComponent, BoxColliderComponent, SpriteComponent>()){
				
				Entity entity(rawEntity, &registry);

				const auto& entityTransform = entity.GetComponent<TransformComponent>();
				auto& entityCollider = entity.GetComponent<BoxColliderComponent>();

				entityCollider.isColliding = false;

				// Plain values only (box by value, layer as int): a collision event can create entities and move components, so a pointer into component storage could dangle.
				const int entityLayer = entity.GetComponent<SpriteComponent>().layer;
				
				colliders.push_back({ entity, BoxAt(entity, entityTransform.position.x, entityTransform.position.y), entityLayer });
			}

			// every pair once: j starts after i
			for (size_t i = 0; i < colliders.size(); ++i) {
				for (size_t j = i + 1; j < colliders.size(); ++j) {
					
					pairsTested++;

					if (Overlaps(colliders[i].box, colliders[j].box)) {
						
						HandlePair(colliders[i], colliders[j], eventBus);
					}
				}
			}
		}


	private:

		struct Collidable {
			Entity entity;
			Box box;
			int layer;
		};

		std::vector<Collidable> colliders;
		
		// Pairs that overlap spatially but should never be treated as a real collision:
		//  - two enemies overlapping each other
		//  - two projectiles overlapping each other, friendly or not
		//  - a friendly (player-fired) projectile overlapping the player ("player" tag)
		//  - an unfriendly (enemy-fired) projectile overlapping an "enemies" group member
		// Uses ProjectileComponent::isFriendly plus the existing Tag/Group data, so it
		// applies the same way regardless of whether the entity came from the Lua level
		// or was spawned at runtime through the ImGui "Create new enemy" tool.
		bool IsCollisionExempt(Entity a, Entity b) {
			if (a.BelongsToGroup("enemies") && b.BelongsToGroup("enemies")) {
				return true;
			}

			bool aIsProjectile = a.HasComponent<ProjectileComponent>();
			bool bIsProjectile = b.HasComponent<ProjectileComponent>();

			if (aIsProjectile && bIsProjectile) {
				return true;
			}

			if (aIsProjectile) {
				bool aIsFriendly = a.GetComponent<ProjectileComponent>().isFriendly;
				if (aIsFriendly && b.HasTag("player")) {
					return true;
				}
				if (!aIsFriendly && b.BelongsToGroup("enemies")) {
					return true;
				}
			}

			if (bIsProjectile) {
				bool bIsFriendly = b.GetComponent<ProjectileComponent>().isFriendly;
				if (bIsFriendly && a.HasTag("player")) {
					return true;
				}
				if (!bIsFriendly && a.BelongsToGroup("enemies")) {
					return true;
				}
			}

			return false;
		}

		void HandlePair(Collidable& a, Collidable& b, std::unique_ptr<EventBus>& eventBus) {
			
			if (IsCollisionExempt(a.entity, b.entity)){
				return;
			}

			const int aLayer = a.layer;
			const int bLayer = b.layer;


			//possible collision events make sure that gound player/enemy wont destroy player/enemy in air just by simple collsion
			bool playerInAir_vs_EnemyInAir =
				(aLayer == 7 && bLayer == 6) ||
				(bLayer == 7 && aLayer == 6);

			bool playerInAir_vs_EnemyOnGround =
				(aLayer == 7 && bLayer == 3) ||
				(bLayer == 7 && aLayer == 3);

			bool playerOnGround_vs_EnemyInAIr =
				(aLayer == 4 && bLayer == 6) ||
				(bLayer == 4 && aLayer == 6);

			if (playerInAir_vs_EnemyInAir){
				a.entity.GetComponent<BoxColliderComponent>().isColliding = true;
				b.entity.GetComponent<BoxColliderComponent>().isColliding = true;
				eventBus->EmitEvent<CollisionEvent>(a.entity, b.entity);
				collisionsEvents++;
			}

			else if (playerInAir_vs_EnemyOnGround || playerOnGround_vs_EnemyInAIr){
				return;
			}

			//collsion on gorund most likely
			else{
				a.entity.GetComponent<BoxColliderComponent>().isColliding = true;
				b.entity.GetComponent<BoxColliderComponent>().isColliding = true;
				eventBus->EmitEvent<CollisionEvent>(a.entity, b.entity);
				collisionsEvents++;
			}
		}
};


#endif

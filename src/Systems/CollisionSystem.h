#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components//SpriteComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class CollisionSystem
{
	public:
		CollisionSystem() = default;

		void Update(Registry& registry, std::unique_ptr<EventBus>& eventBus)
		{
			//Check all the enteties that have the box collider
			//to see if they are colliding with each other
			std::vector<Entity> entities;
			for (auto rawEntity : registry.Raw().view<TransformComponent, BoxColliderComponent, SpriteComponent>())
			{
				entities.emplace_back(rawEntity, &registry);
			}

			for (auto entity : entities)
			{
				auto& collider = entity.GetComponent<BoxColliderComponent>();
				collider.isColliding = false;
			}

			//looping all the entities that system is intrested in
			for (auto i = entities.begin(); i != entities.end(); i++)
			{
				Entity a = *i;
				auto aTransform = a.GetComponent<TransformComponent>();
				auto& aCollider = a.GetComponent<BoxColliderComponent>();
				auto aSprite = a.GetComponent<SpriteComponent>();
				//loop rest eneties on right of i never loop enties on left
				for (auto j = i; j != entities.end(); j++)
				{
					Entity b = *j;
					//bypass if somehow a is b
					if (a == b)
					{
						continue;
					}
					auto bTransform = b.GetComponent<TransformComponent>();
					auto& bCollider = b.GetComponent<BoxColliderComponent>();
					auto bSprite = b.GetComponent<SpriteComponent>();
					//collison check AABB between A B
					bool collisonHappend = CheckAABBCollison(
						aTransform.position.x,
						aTransform.position.y,
						aCollider.width * aTransform.scale.x,
						aCollider.height * aTransform.scale.y,
						bTransform.position.x,
						bTransform.position.y,
						bCollider.width * bTransform.scale.x,
						bCollider.height * bTransform.scale.y
					);

					if (collisonHappend)
					{
						//Friendly fire / self-fire pairs never register as a collision at all
						//(no isColliding flag, no event) - see IsCollisionExempt() below.
						if (IsCollisionExempt(a, b))
						{
							continue;
						}

						//Logger::Log("Entity " + std::to_string(a.GetId()) + " is colliding with " +std::to_string(b.GetId()) );
						//TODO: Event type
						int aLayer = aSprite.layer;
						int bLayer = bSprite.layer;
						//possible collision events make sure that gound player/enemy wont destroy player/enemy in air just by simple collsion
						bool playerInAir_vs_EnemyInAir =
							(aLayer == 7 && bLayer == 6) ||
							(bLayer == 7 && aLayer == 6);
						bool playerInAir_vs_EnemyOnGround =
							(aLayer == 7 && bLayer == 3) ||
							(bLayer == 7 && aLayer == 3);
						bool playerOnGround_vs_EnemyInAIr =
							(aLayer == 4 && bLayer == 6) ||
							(bLayer ==  4 && aLayer == 6);

						if (playerInAir_vs_EnemyInAir)
						{
							aCollider.isColliding = true;
							bCollider.isColliding = true;
							eventBus->EmitEvent<CollisionEvent>(a, b);
						}
						else if (playerInAir_vs_EnemyOnGround || playerOnGround_vs_EnemyInAIr)
						{
							continue;
						}
						//collsion on gorund most likely
						else
						{
							aCollider.isColliding = true;
							bCollider.isColliding = true;
							eventBus->EmitEvent<CollisionEvent>(a, b);
						}
					}
				}
			}
		}

		bool CheckAABBCollison(double aX, double aY, double aW, double aH, double bX, double bY, double bW, double bH)
		{
			return (
				aX < bX + bW &&
				aX + aW > bX &&
				aY < bY + bH &&
				aY + aH > bY
				);
		}

		// Pairs that overlap spatially but should never be treated as a real collision:
		//  - two projectiles overlapping each other, friendly or not
		//  - a friendly (player-fired) projectile overlapping the player ("player" tag)
		//  - an unfriendly (enemy-fired) projectile overlapping an "enemies" group member
		// Uses ProjectileComponent::isFriendly plus the existing Tag/Group data, so it
		// applies the same way regardless of whether the entity came from the Lua level
		// or was spawned at runtime through the ImGui "Create new enemy" tool.
		bool IsCollisionExempt(Entity a, Entity b)
		{
			bool aIsProjectile = a.HasComponent<ProjectileComponent>();
			bool bIsProjectile = b.HasComponent<ProjectileComponent>();

			if (aIsProjectile && bIsProjectile)
			{
				return true;
			}

			if (aIsProjectile)
			{
				bool aIsFriendly = a.GetComponent<ProjectileComponent>().isFriendly;
				if (aIsFriendly && b.HasTag("player"))
				{
					return true;
				}
				if (!aIsFriendly && b.BelongsToGroup("enemies"))
				{
					return true;
				}
			}

			if (bIsProjectile)
			{
				bool bIsFriendly = b.GetComponent<ProjectileComponent>().isFriendly;
				if (bIsFriendly && a.HasTag("player"))
				{
					return true;
				}
				if (!bIsFriendly && a.BelongsToGroup("enemies"))
				{
					return true;
				}
			}

			return false;
		}
};


#endif

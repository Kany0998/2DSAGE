#ifndef DAMAGESYSTEM_H
#define DAMAGESYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class DamageSystem
{
	public:
		DamageSystem() = default;

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
		{
			eventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::onCollision);
		}

		void onCollision(CollisionEvent& ev)
		{
			Entity a = ev.a;
			Entity b = ev.b;

			if (a.BelongsToGroup("projectiles") && b.HasTag("player"))
			{
				OnProjectileHitsPlayer(a, b); //a is projectile b is player
			}

			if(b.BelongsToGroup("projectiles") && a.HasTag("player"))
			{
				OnProjectileHitsPlayer(b, a); //b is projectile a is player
			}

			if(a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies"))
			{
				OnProjectileHitsEnemy(a, b);
			}

			if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies"))
			{
				OnProjectileHitsEnemy(b, a);
			}
		}

		void OnProjectileHitsPlayer(Entity projectile, Entity player)
		{
			auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

			if (!projectileComponent.isFriendly)
			{
				//reduce player health by hitPercentDamage
				auto& health = player.GetComponent<HealthComponent>();

				//substract health of the player by hitPercentDamage of projectile
				health.healthPercentage -= projectileComponent.hitPercentDamage;

				if (health.healthPercentage <= 0)
				{
					player.Kill();
				}

				projectile.Kill();
			}
		}

		void OnProjectileHitsEnemy(Entity projectile, Entity enemy)
		{
			auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

			if (projectileComponent.isFriendly)
			{
				//reduce enemy health by hitPercentDamage
				auto& health = enemy.GetComponent<HealthComponent>();

				//substract health of the enemy by hitPercentDamage of projectile
				health.healthPercentage -= projectileComponent.hitPercentDamage;

				if (health.healthPercentage <= 0)
				{
					enemy.Kill();
				}

				projectile.Kill();
			}
		}
};

#endif
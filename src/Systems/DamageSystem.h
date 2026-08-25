#ifndef DAMAGESYSTEM_H
#define DAMAGESYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/AttributesComponent.h"
#include "../Components/ExperienceRewardComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EntityKilledEvent.h"

class DamageSystem
{
	public:
		// Needs the bus kept alive as a member so deaths can be announced from inside
		// onCollision(): that's an event-callback with a fixed (CollisionEvent&)
		// signature and can't receive the bus as a call parameter.
		explicit DamageSystem(std::unique_ptr<EventBus>& eventBus) : eventBus(eventBus) {}

		// Takes no bus parameter, unlike the other systems: this one already holds the
		// bus it emits deaths on, and accepting a second one here would let a caller
		// subscribe on one bus while announcing on another.
		void SubscribeToEvents()
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
				//reduce player health by projectileDamage
				auto& health = player.GetComponent<HealthComponent>();

				//Kill() only queues the destruction until the next Registry::Update(), so
				//an entity that dropped to 0 stays alive and collidable for the rest of
				//this frame. Without this guard every further hit in that window runs the
				//death branch again.
				if (health.healthPoints <= 0)
				{
					return;
				}

				auto damgeTaken = projectileComponent.projectileDamage;
				//substract health of the player by projectileDamage of projectile
				if (player.HasComponent<AttributesComponent>()) {
					auto& attributes = player.GetComponent<AttributesComponent>();
					
					damgeTaken -= attributes.defensePower;
					if (damgeTaken<=0) {
						damgeTaken = 1; //minimum damage taken is 1
					}
					
				}
				health.healthPoints -= damgeTaken;

				if (health.healthPoints <= 0)
				{
					AnnounceDeath(player, projectileComponent.owner);
					player.Kill();
				}

				projectile.Kill();
			}
		}

		void OnProjectileHitsEnemy(Entity projectile, Entity enemy)
		{
			auto& projectileComponent = projectile.GetComponent<ProjectileComponent>();

			if (projectileComponent.isFriendly && !projectileComponent.haveCollided)
			{
				//reduce enemy health by projectileDamage
				auto& health = enemy.GetComponent<HealthComponent>();

				//Same deferred-kill window as OnProjectileHitsPlayer: an enemy already at
				//0 health is still collidable until the next Registry::Update(). This is
				//the guard that stops a second projectile from running the death branch
				//twice - which matters as soon as dying hands out an experience reward.
				//It also stops corpses from absorbing shots for the rest of the frame.
				if (health.healthPoints <= 0)
				{
					return;
				}

				//substract health of the enemy by projectileDamage of projectile
				auto damgeTaken = projectileComponent.projectileDamage;
				if (enemy.HasComponent<AttributesComponent>()) {
					auto& attributes = enemy.GetComponent<AttributesComponent>();

					damgeTaken  -= attributes.defensePower;
					if (damgeTaken <= 0) {
						damgeTaken = 1; //minimum damage taken is 1
					}
					
				}
				health.healthPoints -= damgeTaken;

				//Everything this function still needs from the projectile is taken care of
				//BEFORE the death is announced. AnnounceDeath() runs every subscriber
				//synchronously, and any one of them that adds a component to a new entity
				//(a loot drop, a death explosion) can reallocate that component's pool -
				//which would turn a write through `projectileComponent` into a write to
				//freed memory. Copying the owner out and flipping the flag first means no
				//reference has to survive the dispatch.
				const Entity killer = projectileComponent.owner;
				projectileComponent.haveCollided = true;

				if (health.healthPoints <= 0)
				{
					AnnounceDeath(enemy, killer);
					enemy.Kill();
				}

				projectile.Kill();
			}
		}

	private:
		// Announces a death on the bus. Emitted before Kill() purely for readability -
		// Kill() only queues the destruction, so the victim's components are readable
		// either way - and the reward is copied into the event here, while the victim
		// is guaranteed to still exist.
		void AnnounceDeath(Entity victim, Entity killer)
		{
			int experienceReward = 0;
			if (victim.HasComponent<ExperienceRewardComponent>())
			{
				experienceReward = victim.GetComponent<ExperienceRewardComponent>().experienceReward;
			}

			eventBus->EmitEvent<EntityKilledEvent>(victim, killer, experienceReward);
		}

		std::unique_ptr<EventBus>& eventBus;
};

#endif
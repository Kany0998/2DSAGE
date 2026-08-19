#ifndef PROGRESSIONSYSTEM_H
#define PROGRESSIONSYSTEM_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../EventBus/EventBus.h"
#include "../Events/EntityKilledEvent.h"
#include "../Components/ProgressionComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ManaComponent.h"
#include <cmath>
#include <string>

// Turns kills into levels. Listens for EntityKilledEvent, banks the reward on
// whoever fired the killing shot, and applies as many level-ups as the new total
// pays for. Anything without a ProgressionComponent (every enemy, for now) is
// simply ignored, so enemies killing each other costs nothing.
class ProgressionSystem
{
	public:
		ProgressionSystem() = default;

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
		{
			eventBus->SubscribeToEvent<EntityKilledEvent>(this, &ProgressionSystem::OnEntityKilled);
		}

		// Fills in a threshold that was never set (0 from Lua, or from a default-built
		// component) using the curve for whatever level the entity actually starts at.
		// This is why the curve isn't duplicated in ProgressionComponent: an entity
		// authored at level 7 needs the level-7 requirement, not the level-1 one.
		// Runs before anything renders, so a bar never sees a zero denominator.
		void Update(Registry& registry)
		{
			for (auto rawEntity : registry.Raw().view<ProgressionComponent>())
			{
				Entity entity(rawEntity, &registry);
				auto& progression = entity.GetComponent<ProgressionComponent>();

				if (progression.experienceToNextLevel <= 0)
				{
					progression.experienceToNextLevel = GetExperienceForLevel(progression, progression.currentLevel);
				}
			}
		}

		void OnEntityKilled(EntityKilledEvent& event)
		{
			Entity killer = event.killer;

			// The killer is a handle stored on the projectile back when it was fired, so
			// it can refer to an entity that has since died - and entt recycles handles,
			// which makes reading components off a stale one undefined rather than null.
			if (!killer.IsAlive())
			{
				return;
			}

			if (!killer.HasComponent<ProgressionComponent>())
			{
				return;
			}

			if (event.experienceReward <= 0)
			{
				return;
			}

			auto& progression = killer.GetComponent<ProgressionComponent>();

			//an entity at the cap stops banking entirely - there is nothing left to buy
			if (progression.currentLevel >= progression.maxLevel)
			{
				return;
			}

			//defence in depth against a threshold that was never set, or was authored as
			//zero/negative in Lua: it would be satisfied by any experience total and drive
			//the loop below straight to maxLevel in one frame
			if (progression.experienceToNextLevel <= 0)
			{
				progression.experienceToNextLevel = GetExperienceForLevel(progression, progression.currentLevel);
			}

			progression.currentExperience += event.experienceReward;

			//a loop rather than a single check: one generous kill can pay for several
			//levels at once. Subtracting the threshold instead of zeroing the total
			//carries the surplus into the next level rather than throwing it away.
			while (progression.currentLevel < progression.maxLevel &&
				progression.currentExperience >= progression.experienceToNextLevel)
			{
				progression.currentExperience -= progression.experienceToNextLevel;
				progression.currentLevel++;
				progression.experienceToNextLevel = GetExperienceForLevel(progression, progression.currentLevel);

				GrantLevelUpRewards(killer, progression);
			}

			//at the cap there is no next level to make progress toward, so park the total
			//on the threshold - an experience bar then reads as full instead of showing
			//progress that can never be completed
			if (progression.currentLevel >= progression.maxLevel)
			{
				progression.currentExperience = progression.experienceToNextLevel;
			}
		}

	private:
		// Experience needed to clear the given level. Deliberately a closed form -
		// a function of the level alone rather than of the previous threshold - so any
		// level's requirement can be asked for directly, without replaying the whole
		// curve from level 1. A recurrence that feeds its own output back in also
		// compounds far faster than it looks: a "small" per-level increase applied on
		// top of an already-grown value overflows an int well before level 100.
		int GetExperienceForLevel(const ProgressionComponent& progression, int level)
		{
			const int levelsGained = (level > 1) ? (level - 1) : 0;

			//double throughout: pow() on ints would truncate the growth rate to 1 and
			//flatten the whole curve
			double requirement = static_cast<double>(progression.experienceBase) * std::pow(static_cast<double>(progression.experienceGrowth), levelsGained);

			//keeps the cast below well inside int range no matter what Lua supplies -
			//a growth of 1.2 over 100 levels alone would otherwise overflow, and an
			//overflowed (negative) threshold is satisfied by everything
			if (requirement < MinExperienceRequirement)
			{
				requirement = MinExperienceRequirement;
			}
			if (requirement > MaxExperienceRequirement)
			{
				requirement = MaxExperienceRequirement;
			}

			return static_cast<int>(requirement);
		}

		// What a level is actually worth. Skill points are the player's to spend; the
		// pool increases are automatic, because vitality and wisdom only govern regen
		// *rate* - without these a level-50 character would carry a level-1 pool.
		void GrantLevelUpRewards(Entity entity, ProgressionComponent& progression)
		{
			progression.unspentSkillPoints += SkillPointsPerLevel;

			if (entity.HasComponent<HealthComponent>())
			{
				auto& health = entity.GetComponent<HealthComponent>();
				health.maxHealthPoints += HealthPerLevel;
				health.healthPoints = health.maxHealthPoints; //levelling up heals to full
			}

			if (entity.HasComponent<ManaComponent>())
			{
				auto& mana = entity.GetComponent<ManaComponent>();
				mana.maxManaPoints += ManaPerLevel;
				mana.manaPoints = mana.maxManaPoints;
			}

			Logger::Log("Entity " + std::to_string(entity.GetId()) + " reached level " + std::to_string(progression.currentLevel) +
				" (" + std::to_string(progression.unspentSkillPoints) + " unspent skill points, next level at " +
				std::to_string(progression.experienceToNextLevel) + " exp)");
		}

		static constexpr int SkillPointsPerLevel = 3;	//points handed out per level, spent into AttributesComponent
		static constexpr int HealthPerLevel = 10;		//added to maxHealthPoints per level
		static constexpr int ManaPerLevel = 10;			//added to maxManaPoints per level

		static constexpr double MinExperienceRequirement = 1.0;
		static constexpr double MaxExperienceRequirement = 1000000000.0;	//1e9, comfortably inside int range
};

#endif

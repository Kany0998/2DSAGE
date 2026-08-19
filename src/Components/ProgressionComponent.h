#ifndef  PROGRESSIONCOMPONENT_H
#define  PROGRESSIONCOMPONENT_H

struct ProgressionComponent
{
	int currentLevel;			// The current level of the entity
	int currentExperience;		// The current experience points of the entity
	int experienceToNextLevel;	// The experience points required to reach the next level
	int unspentSkillPoints;		// The number of unspent skill points available to the entity
	int maxLevel = 100;			// The maximum level the entity can reach
	int experienceBase = 100;	// The experience needed to clear level 1, before any growth is applied

	// How much more each level costs than the one before it: 1.0 means every level
	// costs the same, 1.15 means each one costs 15% more. Kept as a float because a
	// growth rate has no meaningful integer form - the same reason the regen
	// accumulators on HealthComponent/ManaComponent are floats among int stats.
	float experienceGrowth = 1.15f;

	ProgressionComponent(int currentLevel = 1, int currentExperience = 0, int experienceToNextLevel = 0, int unspentSkillPoints = 0, int maxLevel = 100, int experienceBase = 100, float experienceGrowth = 1.15f)
	{
		this->currentLevel = currentLevel;
		this->currentExperience = currentExperience;
		this->unspentSkillPoints = unspentSkillPoints;
		this->maxLevel = maxLevel;
		this->experienceBase = experienceBase;
		this->experienceGrowth = experienceGrowth;

		// 0 means "not set yet" - ProgressionSystem fills it in from the curve on its
		// next Update(), which runs before anything renders. Deriving it here instead
		// would mean duplicating the curve in a component (they are plain data in this
		// codebase) and, worse, getting it wrong for an entity authored part-way up:
		// the level-1 requirement is only correct for a level-1 entity.
		this->experienceToNextLevel = experienceToNextLevel;
	}
};

#endif // ! PROGRESSIONCOMPONENT_H

#ifndef EXPERIENCEREWARDCOMPONENT_H
#define EXPERIENCEREWARDCOMPONENT_H

struct ExperienceRewardComponent
{
	int experienceReward;	// How much experience killing this entity grants

	ExperienceRewardComponent(int experienceReward = 0)
	{
		this->experienceReward = experienceReward;
	}
};

#endif // ! EXPERIENCEREWARDCOMPONENT_H

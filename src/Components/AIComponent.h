#ifndef AICOMPONENT_H
#define AICOMPONENT_H

#include <glm/glm.hpp>
#include <vector>

struct AIComponent {
	
	glm::vec2 spawnPoint;
	double detectionRange;	//Stored In pixels convent from tiles
	double stopRange;		//Stored In pixels convent from tiles
	double movementSpeed;

	std::vector<int> path;
	int waypointIndex;
	double repathTimer;
	int lastTargetCell;
	bool isChasing;
	bool scriptControlsVelocity;

	AIComponent(glm::vec2 spawnPoint = glm::vec2(0.0, 0.0), double detectionRange = 0, double stopRange = 0, double movementSpeed = 150.0) {
		
		this->spawnPoint = spawnPoint;
		this->detectionRange = detectionRange;
		this->stopRange = stopRange;
		this->movementSpeed = movementSpeed;
		this->waypointIndex = 0;
		this->repathTimer = 0;
		this->lastTargetCell = -1;
		this->isChasing = false;
		this->scriptControlsVelocity = false;

	}
};

#endif // AICOMPONENT_H
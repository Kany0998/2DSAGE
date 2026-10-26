#ifndef AICOMPONENT_H
#define AICOMPONENT_H

#include "../Pathfinding/Pathfinder.h"
#include "../TileMap/TileMap.h"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

enum class AIState {
	Patrol,
	Chase,
	Return
};


inline constexpr double aggroOnHitSeconds = 5.0; //Being shot pulls an enemy for this long, regardless of how far the shooter is

struct AIComponent {
	
	glm::vec2 spawnPoint;	//Center of the Sprite
	double detectionRange;	//Stored In pixels convent from tiles
	double stopRange;		//Stored In pixels convent from tiles
	double leashRange;		//Stored In pixels convent from tiles
	double movementSpeed;
	double patrolRadius;	//Stored In pixels convent from tiles
	double patrolPause;		//Stored In seconds

	std::vector<int> path;
	int waypointIndex;
	double repathTimer;
	int lastTargetCell;
	AIState state;
	bool scriptControlsVelocity;

	int patrolTargetCell;
	double patrolPauseTimer;
	int failedSearchCount;
	double aggroCooldown;
	double aggroTimer;

	std::vector<int> laneCells;


	AIComponent(glm::vec2 spawnPoint = glm::vec2(0.0, 0.0), double detectionRange = 0, double stopRange = 0,double leashRange = 0,
		double movementSpeed = 150.0, double patrolRadius = 0, double patrolPause = 0) {
		
		this->spawnPoint = spawnPoint;
		this->detectionRange = detectionRange;
		this->stopRange = stopRange;
		this->leashRange = leashRange;
		this->movementSpeed = movementSpeed;
		this->patrolRadius = patrolRadius;
		this->patrolPause = patrolPause;
		this->waypointIndex = 0;
		this->repathTimer = 0.0;
		this->lastTargetCell = -1;
		this->state = AIState::Patrol;
		this->scriptControlsVelocity = false;
		this->patrolTargetCell = -1;
		this->patrolPauseTimer = 0.0;
		this->failedSearchCount = 0.0;
		this->aggroCooldown = 0.0;
		this->aggroTimer = 0.0;

	}


};

inline void EnterAIState(AIComponent& ai, AIState next) {
	ai.state = next;
	ai.path.clear();
	ai.waypointIndex = 0;
	ai.lastTargetCell = -1;
	ai.repathTimer = 0.0;
	ai.failedSearchCount = 0;
	ai.laneCells.clear();
	if (next == AIState::Patrol) {
		ai.patrolTargetCell = -1;
	}
}

inline SearchBounds BoundsForEnemy(const AIComponent& enemyAI, const TileMap& tileMap) {
	const int spawnCol = tileMap.colAt(enemyAI.spawnPoint.x);
	const int spawnRow = tileMap.rowAt(enemyAI.spawnPoint.y);

	const int intrestRadius = static_cast<int>(std::ceil((enemyAI.leashRange + enemyAI.detectionRange) / tileMap.TileWorldSize()) + 2);

	SearchBounds searchBounds;
	searchBounds.minCol = std::max(0, spawnCol - intrestRadius);
	searchBounds.maxCol = std::min(tileMap.Cols() - 1, spawnCol + intrestRadius);
	searchBounds.minRow = std::max(0, spawnRow - intrestRadius);
	searchBounds.maxRow = std::min(tileMap.Rows() - 1, spawnRow + intrestRadius);
	return searchBounds;
}

#endif // AICOMPONENT_H
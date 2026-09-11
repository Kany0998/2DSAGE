#ifndef  MOVEMENTTYPECOMPONENT_H
#define  MOVEMENTTYPECOMPONENT_H

#include "../TileMap/MovementType.h"

struct MovementTypeComponent
{
	int movementType = MovementType_Ground; // Default to ground movement

	MovementTypeComponent(int movementType = MovementType_Ground)
	{
		this->movementType = movementType;
	}
};


#endif // MOVEMENTTYPECOMPONENT_H

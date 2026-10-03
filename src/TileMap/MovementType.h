#ifndef  MOVEMENTTYPE_H
#define  MOVEMENTTYPE_H

// The movement type of an entity, used to determine which tiles it can traverse.
// This is a bitmask, so an entity can have multiple movement types at once.

enum MovementType {
	MovementType_Ground = 1,
	MovementType_Flying = 2,
	MovementType_Swimming = 4,
	MovementType_Projectile = 8
};


#endif // MOVEMENTTYPE_H
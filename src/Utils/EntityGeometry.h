#ifndef ENTITYGEOMETRY_H
#define ENTITYGEOMETRY_H

#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../ECS/ECS.h"
#include <glm/glm.hpp>

//The point every system agrees on: terrain collision, the pathfinder, detection
//ranges and aiming all measure from the centre of the sprite, not its corner
inline glm::vec2 EntityCenter(const TransformComponent& transform, const SpriteComponent& sprite) {
	return glm::vec2(
		transform.position.x + sprite.width * transform.scale.x / 2.0,
		transform.position.y + sprite.height * transform.scale.y / 2.0
	);
}

//Collider box not the sprite
struct Box { double left, top, width, height; };

//caller must check the collide
inline Box BoxAt(Entity e, double x, double y) {
	const auto& entityTransform = e.GetComponent<TransformComponent>();
	const auto& entityCollider = e.GetComponent<BoxColliderComponent>();

	const double left = x + entityCollider.offset.x;
	const double top = y + entityCollider.offset.y;
	const double width = entityCollider.width * entityTransform.scale.x;
	const double height = entityCollider.height * entityTransform.scale.y;

	return Box{ left,top,width,height };
}

inline bool Overlaps(const Box& a, const Box& b) {

	return (
		a.left < b.left + b.width
		&& a.left + a.width > b.left
		&& a.top < b.top + b.height
		&& a.top + a.height > b.top
		);
	
}



#endif // !ENTITYGEOMETRY_H

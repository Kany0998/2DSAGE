#ifndef ENTITYGEOMETRY_H
#define ENTITYGEOMETRY_H

#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <glm/glm.hpp>

//The point every system agrees on: terrain collision, the pathfinder, detection
//ranges and aiming all measure from the centre of the sprite, not its corner
inline glm::vec2 EntityCenter(const TransformComponent& transform, const SpriteComponent& sprite) {
	return glm::vec2(
		transform.position.x + sprite.width * transform.scale.x / 2.0,
		transform.position.y + sprite.height * transform.scale.y / 2.0
	);
}



#endif // !ENTITYGEOMETRY_H

#ifndef RIGIDBODYCOMPONENT_H
#define RIGIDBODYCOMPONENT_H

#include <glm/glm.hpp>

struct RigidBodyComponent
{
	glm::vec2 velocity;

	//What the entity asks for is velocity; this is what the surface actually gives it.
	//Equal on normal ground, lagging behind on ice. Only MovementSystem writes it
	glm::vec2 actualVelocity = glm::vec2(0.0, 0.0);

	RigidBodyComponent(glm::vec2 velocity = glm::vec2(0.0, 0.0))
	{
		this->velocity = velocity;
		this->actualVelocity = glm::vec2(0.0, 0.0);
	}
};

#endif
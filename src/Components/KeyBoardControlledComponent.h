#ifndef KEYBOARDCONTROLLEDCOMPONENT
#define KEYBOARDCONTROLLEDCOMPONENT

#include <glm/glm.hpp>

struct KeyBoardControlledComponent
{
	bool diagnalMovement;
	glm::vec2 upVelocity;
	glm::vec2 rightVelocity;
	glm::vec2 downVelocity;
	glm::vec2 leftVelocity;

	KeyBoardControlledComponent(glm::vec2 upVelocity = glm::vec2(0), glm::vec2 rightVelocity = glm::vec2(0), glm::vec2 downVelocity = glm::vec2(0), glm::vec2 leftVelocity = glm::vec2(0),bool diagnalMovement = false)
	{
		this->upVelocity = upVelocity;
		this->rightVelocity = rightVelocity;
		this->downVelocity = downVelocity;
		this->leftVelocity = leftVelocity;
		this->diagnalMovement = diagnalMovement;
	}
};


#endif

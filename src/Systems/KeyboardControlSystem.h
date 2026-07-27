#ifndef KEYBOARDCONTROLSYSTEM_H
#define KEYBOARDCONTROLSYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/KeyBoardControlledComponent.h"
#include "../Components/TransformComponent.h"

class KeyboardControlSystem
{
public:
	KeyboardControlSystem() = default;

	void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
	{
		eventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::onPress);
	}

	void onPress(KeyPressedEvent& ev)
	{

	}

	void Update(Registry& registry)
	{
		const Uint8* keystate = SDL_GetKeyboardState(NULL);
		bool up = keystate[SDL_SCANCODE_W];
		bool right = keystate[SDL_SCANCODE_D];
		bool down = keystate[SDL_SCANCODE_S];
		bool left = keystate[SDL_SCANCODE_A];
		bool usedDiagonal = false;
		//chaning the sprite and speed of entity
		for (auto rawEntity : registry.Raw().view<KeyBoardControlledComponent, RigidBodyComponent, SpriteComponent, TransformComponent>())
		{
			Entity entity(rawEntity, &registry);

			const auto keyboardControlled = entity.GetComponent<KeyBoardControlledComponent>();
			auto& sprite = entity.GetComponent<SpriteComponent>();
			auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
			auto& transform = entity.GetComponent<TransformComponent>();


			if (keyboardControlled.diagnalMovement)
			{
				if (up && right)
				{
					rigidBody.velocity.y = keyboardControlled.upVelocity.y;
					rigidBody.velocity.x = keyboardControlled.rightVelocity.x;
					sprite.srcRect.y = sprite.height * 0;
					transform.rotation = 45.0;
					usedDiagonal = true;
				}

				else if (down && right)
				{
					rigidBody.velocity.y = keyboardControlled.downVelocity.y;
					rigidBody.velocity.x = keyboardControlled.rightVelocity.x;
					sprite.srcRect.y = sprite.height * 1;
					transform.rotation = 45.0;
					usedDiagonal = true;
				}

				else if (down && left)
				{
					rigidBody.velocity.y = keyboardControlled.downVelocity.y;
					rigidBody.velocity.x = keyboardControlled.leftVelocity.x;
					sprite.srcRect.y = sprite.height * 2;
					transform.rotation = 45.0;
					usedDiagonal = true;
				}
				else if (up && left)
				{
					rigidBody.velocity.y = keyboardControlled.upVelocity.y;
					rigidBody.velocity.x = keyboardControlled.leftVelocity.x;
					sprite.srcRect.y = sprite.height * 3;
					transform.rotation = 45.0;
					usedDiagonal = true;
				}
			}

			if (!usedDiagonal)
			{
				if (up)
				{
					rigidBody.velocity = keyboardControlled.upVelocity;
					sprite.srcRect.y = sprite.height * 0;
					transform.rotation = 0.0;
				}
				else if (right)
				{
					rigidBody.velocity = keyboardControlled.rightVelocity;
					sprite.srcRect.y = sprite.height * 1;
					transform.rotation = 0.0;
				}
				else if (down)
				{
					rigidBody.velocity = keyboardControlled.downVelocity;
					sprite.srcRect.y = sprite.height * 2;
					transform.rotation = 0.0;
				}
				else if (left)
				{
					rigidBody.velocity = keyboardControlled.leftVelocity;
					sprite.srcRect.y = sprite.height * 3;
					transform.rotation = 0.0;
				}
				else
				{
					rigidBody.velocity = glm::vec2(0, 0);
					transform.rotation = 0.0;
				}
			}

		}
	}
};

#endif
#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Logger/Logger.h"	
#include "../ECS/ECS.h"
#include <tuple>

//delcaration of c++ function that will be binded with lua

std::tuple<double, double> GetEntityPosisiton(Entity entity)
{
	if (entity.HasComponent<TransformComponent>())
	{
		const auto transform = entity.GetComponent<TransformComponent>();
		return std::make_tuple(transform.position.x, transform.position.y);
	}
	else
	{
		Logger::Err("You trying to get positon of entity that has no transform component");
		return std::make_tuple(0.0, 0.0);
	}
}

std::tuple<double, double> GetEntityVelocity(Entity entity)
{
	if (entity.HasComponent<RigidBodyComponent>())
	{
		const auto rigid = entity.GetComponent<RigidBodyComponent>();
		return std::make_tuple(rigid.velocity.x, rigid.velocity.y);
	}
	else
	{
		Logger::Err("You trying to get velocity of entity that has no rigidbody component");
		return std::make_tuple(0.0, 0.0);
	}
}

void SetEntityPosition(Entity entity, double x, double y)
{
	if (entity.HasComponent<TransformComponent>())
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.position.x = x;
		transform.position.y = y;
	}
	else
	{
		Logger::Err("Trying to set position of entity that do not have transform component");
	}
}

void SetEntityVelocity(Entity entity, double x, double y)
{
	if (entity.HasComponent<RigidBodyComponent>())
	{
		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		rigidbody.velocity.x = x;
		rigidbody.velocity.y = y;
	}
	else
	{
		Logger::Err("Trying to set velocity of entity that do not have rigidbody component");
	}
}

void SetEntityRotation(Entity entity, double angle)
{
	if (entity.HasComponent<TransformComponent>())
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.rotation = angle;
	}
	else
	{
		Logger::Err("Trying to set rotation of entity that do not have transform component");
	}
}

void SetAnimationFrame(Entity entity, int frame)
{
	if (entity.HasComponent<AnimationComponent>())
	{
		auto& animation = entity.GetComponent<AnimationComponent>();
		animation.currentFrame = frame;
	}
	else
	{
		Logger::Err("Trying to set animation frame of entity that do not have animation component");
	}
}

void SetProjectileVelocity(Entity entity, double x, double y)
{
	if (entity.HasComponent<ProjectileEmitterComponent>())
	{
		auto& projectile = entity.GetComponent<ProjectileEmitterComponent>();
		projectile.projectileVelocity.x = x;
		projectile.projectileVelocity.y = y;
	}
	else
	{
		Logger::Err("Trying to set velocity of projectile of entity that do not have projectileEmitterComponent component");
	}
}

class ScriptSystem : public System
{
	public:
		ScriptSystem()
		{
			RequireComponent<ScriptComponent>();
		}

		void CreateLuaBindings(sol::state& lua)
		{
			//Create the entity user type so lua knows what is
			lua.new_usertype<Entity>(
				"entity",
				"get_id", &Entity::GetId,
				"destroy", &Entity::Kill,
				"has_tag", &Entity::HasTag,
				"belongs_to_group", &Entity::BelongsToGroup
				);

			//Create all the bindings between c++ and lua
			lua.set_function("get_position", GetEntityPosisiton);
			lua.set_function("get_velocity", GetEntityVelocity);
			lua.set_function("set_position", SetEntityPosition);
			lua.set_function("set_velocity", SetEntityVelocity);
			lua.set_function("set_rotation", SetEntityRotation);
			lua.set_function("set_animation_frame", SetAnimationFrame);
			lua.set_function("set_projectile_velocity", SetProjectileVelocity);
		}

		void Update(double deltaTime, int ellapsedTime)
		{
			//loop all the entites with script component and invoke their lua function
			for (auto entity : GetSystemEntities())
			{
				const auto script = entity.GetComponent<ScriptComponent>();
				script.func(entity, deltaTime, ellapsedTime);
			}
		}
};


#endif
#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/AIComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Utils/Projectile.h"
#include "../Utils/EntityGeometry.h"
#include "../TileMap/TileMap.h"
#include "../AssetStore/AssetStore.h"
#include "../Logger/Logger.h"	
#include "../ECS/ECS.h"
#include <tuple>
#include <random>

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

std::tuple<double, double> GetEntityCenter(Entity entity) {
	if (entity.HasComponent<TransformComponent>() && entity.HasComponent<SpriteComponent>()) {
		glm::vec2 center = EntityCenter(entity.GetComponent<TransformComponent>(), entity.GetComponent<SpriteComponent>());
		return std::make_tuple(center.x, center.y);
	}
	else if (entity.HasComponent<TransformComponent>()) {
		return std::make_tuple(entity.GetComponent<TransformComponent>().position.x, entity.GetComponent<TransformComponent>().position.y);
	}
	else {
		Logger::Err("get_center: your entity do not have transform nor sprite return (0.0 , 0.0)");
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

void SetEntityAIPaused(Entity entity, bool paused) {
	if (entity.HasComponent<AIComponent>()) {
		auto& ai = entity.GetComponent<AIComponent>();
		ai.scriptControlsVelocity = paused;
	}
	else {
		Logger::Err("Trying to pause AI of entity that does not have an AI component");
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



class ScriptSystem
{
	public:
		ScriptSystem() = default;

		void CreateLuaBindings(sol::state& lua, Registry& registry, AssetStore& assetStore, const TileMap& tileMap)
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
			lua.set_function("get_center", GetEntityCenter);
			lua.set_function("set_position", SetEntityPosition);
			lua.set_function("set_velocity", SetEntityVelocity);
			lua.set_function("set_rotation", SetEntityRotation);
			lua.set_function("set_animation_frame", SetAnimationFrame);
			lua.set_function("set_projectile_velocity", SetProjectileVelocity);
			lua.set_function("set_ai_paused", SetEntityAIPaused);

			//roll_damage(caster, min, max): rolled and scaled by the caster's attack, like weapon shots
			lua.set_function("roll_damage", [this](Entity caster, int minDamage, int maxDamage) {return RollScaledDamage(caster, minDamage, maxDamage, rng);});

			//spawn_projectile{owner, x, y, velocity_x, velocity_y, damage, range_tiles | lifetime_seconds, texture}
			lua.set_function("spawn_projectile", [&registry, &assetStore, &tileMap](sol::table args) {
					sol::optional<Entity> owner = args["owner"];

					if (!owner || !owner->IsAlive()) {
						Logger::Err("spawn_projectile: the owner is missing or is not alive");
						return;
					}
					float x = args["x"].get_or(0.0f);
					float y = args["y"].get_or(0.0f);
					float velocityX = args["velocity_x"].get_or(0.0f);
					float velocityY = args["velocity_y"].get_or(0.0f);
					int damage = static_cast<int>(std::lround(args["damage"].get_or(1.0)));
					std::string texture = args["texture"].get_or(std::string("bullet-texture"));
					int lifetimeMs = static_cast<int>(std::lround(args["lifetime_seconds"].get_or(3.0) * 1000.0));

					const glm::vec2 velocity(velocityX, velocityY);
					const float speed = glm::length(velocity);

					sol::optional<double> rangeTiles = args["range_tiles"];
					if (rangeTiles && speed > 0.0f) {
						lifetimeMs = static_cast<int>(std::lround(*rangeTiles * tileMap.TileWorldSize() / speed * 1000.0));
					}

					SpawnProjectile(registry, assetStore, *owner, glm::vec2(x, y), velocity, damage, lifetimeMs, texture);
				});
		}

		void Update(Registry& registry, double deltaTime, int ellapsedTime)
		{
			//loop all the entites with script component and invoke their lua function
			for (auto rawEntity : registry.Raw().view<ScriptComponent>())
			{
				Entity entity(rawEntity, &registry);
				const auto script = entity.GetComponent<ScriptComponent>();
				script.func(entity, deltaTime, ellapsedTime);
			}
		}

	private:
		std::mt19937 rng{ std::random_device{}() };
};


#endif
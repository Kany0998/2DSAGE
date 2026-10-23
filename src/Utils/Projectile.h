#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "../ECS/ECS.h"
#include "../Components/AttributesComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../AssetStore/AssetStore.h"
#include <glm/glm.hpp>
#include <SDL.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>


//rolled between min and max, scaled by the owner's attack
//round to the nearest whole number, and never less than 1
inline int RollScaledDamage(Entity owner, int minDamage, int maxDamage, std::mt19937& rng) {

	
	if (minDamage > maxDamage) {
		std::swap(minDamage, maxDamage);
	}
	std::uniform_int_distribution<int> roll(minDamage, maxDamage);
	int damage = roll(rng);
	float scaled = static_cast<float>(damage);


	if (owner.HasComponent<AttributesComponent>()) {
		const auto& attributes = owner.GetComponent<AttributesComponent>();
		scaled = damage * (1.0f + attributes.Effective(Skill_Attack) / 25.0f);
	}

	return std::max(1, static_cast<int>(std::lround(scaled)));
}


inline Entity SpawnProjectile(Registry& registry, AssetStore& assetStore, Entity owner, glm::vec2 center, 
	glm::vec2 velocity, int damage, int lifetimeMs, const std::string& textureId) {

	//Take scale of owner for projectile
	glm::vec2 scale(1.0f, 1.0f);
	if (owner.HasComponent<TransformComponent>()) {
		scale = owner.GetComponent<TransformComponent>().scale;
	}

	//check if projectile is friendly true by defult so player wont kill himself when missing
	bool isFriendly = true;
	if (owner.HasComponent<ProjectileEmitterComponent>()) {
		isFriendly = owner.GetComponent<ProjectileEmitterComponent>().isFriendly;
	}

	int width = 4;
	int height = 4;

	//Bullet look and size from its texture; a missing texture falls back to bullet-texture

	std::string textureName = textureId;
	SDL_Texture* texture = assetStore.GetTexture(textureName);
	if (texture == nullptr) {
		textureName = "bullet-texture";
		texture = assetStore.GetTexture(textureName);
	}

	if (texture != nullptr) {
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
	}

	const double rotation = glm::degrees(std::atan2(velocity.y, velocity.x)) - assetStore.GetTextureFacing(textureName);

	//square hitbox centred on the sprite - colliders don't rotate, the sprite does
	const int hitboxSize = std::max(width, height);
	const glm::vec2 offset(
		(width - hitboxSize) / 2.0f * scale.x,
		(height - hitboxSize) / 2.0f *scale.y);


	center -= glm::vec2(width * scale.x / 2.0f, height * scale.y / 2.0f);

	Entity projectile = registry.CreateEntity();
	projectile.Group("projectiles");
	projectile.AddComponent<TransformComponent>(center, glm::vec2(scale.x, scale.y), rotation);
	projectile.AddComponent<RigidBodyComponent>(velocity);
	projectile.AddComponent<SpriteComponent>(textureName, width, height, 8);
	projectile.AddComponent<BoxColliderComponent>(hitboxSize, hitboxSize, offset);
	//`owner` is recorded as the owner so whatever this shot kills can be
	//credited back to the shooter
	projectile.AddComponent<ProjectileComponent>(isFriendly, damage, lifetimeMs, owner);
	projectile.AddComponent<MovementTypeComponent>(MovementType_Projectile);

	return projectile;
}

#endif // !PROJECTILE_H

#ifndef ECS_H
#define ECS_H
#include <entt/entt.hpp>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include "../Logger/Logger.h"

class Registry;

// Lightweight handle bundling an entt entity with the registry that owns it.
// Keeps the old custom-ECS call-site API (AddComponent/GetComponent/Tag/Group/Kill)
// so systems and Lua bindings don't need to change, while storage is now entt
// (sparse-set component pools instead of the old hand-rolled Pool<T>).
class Entity
{
	private:
		entt::entity entityHandle{ entt::null };

	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Registry* registry) : entityHandle(entityHandle), registry(registry) {}
		Entity(const Entity& entity) = default;

		entt::entity GetHandle() const { return entityHandle; }
		int GetId() const;
		void Kill();

		//Mange entity tags and groups
		void Tag(const std::string& tag);
		bool HasTag(const std::string& tag) const;
		void Group(const std::string& group);
		bool BelongsToGroup(const std::string& group) const;


		//operators overload
		Entity& operator = (const Entity& other) = default;
		bool operator == (const Entity& other) const { return entityHandle == other.entityHandle; }
		bool operator != (const Entity& other) const { return entityHandle != other.entityHandle; }
		bool operator < (const Entity& other) const { return entityHandle < other.entityHandle; }
		bool operator > (const Entity& other) const { return entityHandle > other.entityHandle; }

		//Hold a pointer to entity;s owner registry
		template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
		template <typename TComponent> void RemoveComponent();
		template <typename TComponent> bool HasComponent() const;
		template <typename TComponent> TComponent& GetComponent() const;

		class Registry* registry = nullptr;
};

// Owns the entt::registry (component storage) plus the tag/group bookkeeping
// the custom ECS used to provide, since entt has no built-in equivalent for those.
// Systems no longer register themselves here (no signatures/pools to maintain) -
// they query entt views directly through Raw().
class Registry
{
	private:
		entt::registry enttRegistry;

		//Entity tags (one tag name per entity)
		std::unordered_map<std::string, entt::entity> entityPerTag;
		std::unordered_map<entt::entity, std::string> tagPerEntity;

		//Entity groups (multiple groups per entity)
		std::unordered_map<std::string, std::set<entt::entity>> entitiesPerGroup;
		std::unordered_map<entt::entity, std::string> groupPerEntity;

		//Entities flagged to be destroyed at the next Update(), mirroring the old
		//deferred-kill behavior so systems can safely kill entities mid-frame
		//(e.g. from a collision event handler) without invalidating views/iterators.
		std::vector<entt::entity> entitiesToBeKilled;

	public:
		Registry() { Logger::Log("Registry Constructor"); }
		~Registry() { Logger::Log("Registry Destructor"); }

		//Direct access to the underlying entt registry, e.g. registry.Raw().view<...>()
		entt::registry& Raw() { return enttRegistry; }

		//Destroys entities that were killed since the last Update()
		void Update();

		//entity managment
		Entity CreateEntity();
		void KillEntity(Entity entity);

		//adding/removing components straight to/from the entt component pools
		template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
		template <typename TComponent> void RemoveComponent(Entity entity);
		template <typename TComponent> bool HasComponent(Entity entity) const;
		template <typename TComponent> TComponent& GetComponent(Entity entity);

		//tags management
		void TagEntity(Entity entity, const std::string& tag);
		bool EntityHasTag(Entity entity, const std::string& tag) const;
		Entity GetEntityByTag(const std::string& tag);
		void RemoveEntityTag(Entity entity);

		//groups management
		void GroupEntity(Entity entity, const std::string& group);
		bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
		std::vector<Entity> GetEntitiesByGroup(const std::string& group);
		void RemoveEntityGroup(Entity entity);
};




/// ///////////TComponent Management ///////////////

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	enttRegistry.emplace<TComponent>(entity.GetHandle(), std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	enttRegistry.remove<TComponent>(entity.GetHandle());
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const
{
	return enttRegistry.all_of<TComponent>(entity.GetHandle());
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity)
{
	return enttRegistry.get<TComponent>(entity.GetHandle());
}


/////////////////////////Entity//////////////////////////////

template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args)
{
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}
template <typename TComponent>
void Entity::RemoveComponent()
{
	registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const
{
	return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const
{
	return registry->GetComponent<TComponent>(*this);
}
#endif

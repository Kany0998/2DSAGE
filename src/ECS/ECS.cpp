#include "ECS.h"
#include "../Logger/Logger.h"

int Entity::GetId() const
{
	return static_cast<int>(entt::to_integral(entityHandle));
}

void Entity::Kill()
{
	registry->KillEntity(*this);
}

bool Entity::IsAlive() const
{
	//a default-constructed Entity has no registry at all, so check that first
	return registry != nullptr && registry->Raw().valid(entityHandle);
}

void Entity::Tag(const std::string& tag)
{
	registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const
{
	return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group)
{
	registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const
{
	return registry->EntityBelongsToGroup(*this, group);
}

Entity Registry::CreateEntity()
{
	entt::entity handle = enttRegistry.create();
	Entity entity(handle, this);

	Logger::Log("Entity created with ID: " + std::to_string(entity.GetId()));

	return entity;
}

void Registry::KillEntity(Entity entity)
{
	entitiesToBeKilled.insert(entity.GetHandle());
}

void Registry::Update()
{
	for (entt::entity handle : entitiesToBeKilled)
	{
		//Guard against a handle that's already been destroyed (defense in depth,
		//on top of entitiesToBeKilled being a set) - calling destroy() twice on
		//the same entity trips an entt assertion.
		if (!enttRegistry.valid(handle))
		{
			continue;
		}

		Entity entity(handle, this);

		//remove any traces of that entity from the tag/group maps before it is destroyed
		RemoveEntityTag(entity);
		RemoveEntityGroup(entity);

		enttRegistry.destroy(handle);

		Logger::Log("Entity destroyed with ID: " + std::to_string(entity.GetId()));
	}
	entitiesToBeKilled.clear();
}

void Registry::TagEntity(Entity entity, const std::string& tag)
{
	entityPerTag.emplace(tag, entity.GetHandle());
	tagPerEntity.emplace(entity.GetHandle(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const
{
	if (tagPerEntity.find(entity.GetHandle()) == tagPerEntity.end())
	{
		return false;
	}
	return entityPerTag.find(tag)->second == entity.GetHandle();
}

Entity Registry::GetEntityByTag(const std::string& tag)
{
	return Entity(entityPerTag.at(tag), this);
}

void Registry::RemoveEntityTag(Entity entity)
{
	auto taggedEntity = tagPerEntity.find(entity.GetHandle());
	if (taggedEntity != tagPerEntity.end())
	{
		auto tag = taggedEntity->second;
		entityPerTag.erase(tag);
		tagPerEntity.erase(taggedEntity);
	}
}

void Registry::GroupEntity(Entity entity, const std::string& group)
{
	entitiesPerGroup.emplace(group, std::set<entt::entity>());
	entitiesPerGroup[group].insert(entity.GetHandle());
	groupPerEntity.emplace(entity.GetHandle(), group);
}

bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const
{
	auto groupEntities = entitiesPerGroup.find(group);
	if (groupEntities == entitiesPerGroup.end())
	{
		return false;
	}

	return groupEntities->second.find(entity.GetHandle()) != groupEntities->second.end();
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group)
{
	auto& setOfEntities = entitiesPerGroup.at(group);
	std::vector<Entity> result;
	result.reserve(setOfEntities.size());
	for (entt::entity handle : setOfEntities)
	{
		result.emplace_back(handle, this);
	}
	return result;
}

void Registry::RemoveEntityGroup(Entity entity)
{
	auto groupedEntity = groupPerEntity.find(entity.GetHandle());
	if (groupedEntity != groupPerEntity.end())
	{
		auto group = entitiesPerGroup.find(groupedEntity->second);
		if(group != entitiesPerGroup.end())
		{
			group->second.erase(entity.GetHandle());
		}
		groupPerEntity.erase(groupedEntity);
	}
}

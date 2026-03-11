#ifndef ECS_H
#define ECS_H
#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include<set>
#include <memory>
#include <deque>
#include "../Logger/Logger.h"

const unsigned int MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
	protected:
		static int nextId;
};

// it is used to assign uniqe IDs to each component type
template <typename T>
class Component: public IComponent 
{
	//returns a unique ID for each component type
	public:
		static int GetId()
		{
			static auto id = nextId++;
			return id;
		}
};

class Entity 
{
	private:
		int id;
	public:
		Entity(int id) : id(id) {}
		Entity(const Entity& entity) = default;
		int GetId() const;
		void Kill();

		//Mange entity tags and groups
		void Tag(const std::string& tag);
		bool HasTag(const std::string& tag) const;
		void Group(const std::string& group);
		bool BelongsToGroup(const std::string& group) const;


		//operators overload
		Entity& operator = (const Entity& other) = default;
		bool operator == (const Entity& other) const { return id == other.id; }
		bool operator != (const Entity& other) const { return id != other.id; }
		bool operator < (const Entity& other) const { return id < other.id; }
		bool operator > (const Entity& other) const { return id > other.id; }

		//Hold a pointer to entity;s owner registry
		template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
		template <typename TComponent> void RemoveComponent();
		template <typename TComponent> bool HasComponent() const;
		template <typename TComponent> TComponent& GetComponent() const;

		class Registry* registry;
};

class System 
{
	private:
		Signature componentSignature;
		std::vector<Entity> entities;

	public:
		System() = default;
		~System() = default;

		void AddEntityToSystem(Entity entity);
		void RemoveEntityFromSystem(Entity entity);
		std::vector<Entity> GetSystemEntities() const;
		const Signature& GetComponetsSignature() const;

		//defines a commpontent type that enties must have to be considered by this system
		template <typename TComponent> void RequireComponent();


};


//Pool    a pool is just a vector (contiguous data) of objects of T
class IPool
{
	public:
		virtual ~IPool() = default;
		virtual void RemoveEntityFromPool(int entityId) = 0;
};


template <typename T>
class Pool : public IPool
{
	private:
		//we keep track of vector of objcets and current number of elements
		std::vector<T> data;
		int size;

		//Healper maps to keep track of entity ids per index so the vector is always packed
		std::unordered_map<int, int> entityIdToIndex;
		std::unordered_map<int, int> indexToEntityId;

	public:
		Pool(int capacity = 100)
		{
			size = 0;
			data.resize(capacity);
		}
		virtual ~Pool() = default;

		bool IsEmpty() const
		{
			return size == 0;
		}

		int GetSize() const
		{
			return size;
		}

		void Resize(int newSize)
		{
			data.resize(newSize);
		}

		void Clear()
		{
			data.clear();
			size = 0;
		}

		void Add(T object)
		{
			data.push_back(object);
		}

		void Set(int entityId, T object)
		{
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end())
			{
				//if element already exists , replace the component object
				int index = entityIdToIndex[entityId];
				data[index] = object;
			}

			else
			{
				//adding new object and keep track of the entity ids and their vector index
				int index = size;
				entityIdToIndex.emplace(entityId, index);
				indexToEntityId.emplace(index, entityId);
				if(index >= data.capacity())
				{
					data.resize(data.capacity() * 2);
				}
				data[index] = object;
				size++;
			}
		}

		void Remove(int entityId)
		{
			//copy the last element to the removed element's place to keep the vector packed
			int indexOfRemovedEntity = entityIdToIndex[entityId];
			int indexOfLastElement = size - 1;
			data[indexOfRemovedEntity] = data[indexOfLastElement];

			//Update the index-enetiy maps to point to correct indices
			int entityIdOfLastElement = indexToEntityId[indexOfLastElement];
			entityIdToIndex[entityIdOfLastElement] = indexOfRemovedEntity;
			indexToEntityId[indexOfRemovedEntity] = entityIdOfLastElement;

			entityIdToIndex.erase(entityId);
			indexToEntityId.erase(indexOfLastElement);

			size--;
		}

		void RemoveEntityFromPool(int entityId) override
		{
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end())
			{
				Remove(entityId);
			}
		}

		T& Get(int entityId)
		{
			int index = entityIdToIndex[entityId];
			return static_cast<T&>(data[index]);
		}

		T& operator [](unsigned int index)
		{
			return data[index];
		}
};

//managies creation and distruction of entites , add system and compnents
class Registry 
{
	private:
		int numberOfEntities = 0;

		//vector of compnets pool each pool contains all the data for a specific component type
		//vector index is the component type ID
		//Pool index = entity ID
		std::vector<std::shared_ptr<IPool>> componentPools;

		//vector of component signatures per entity saying which compnent is used on for each entity
		//vector index = entity ID
		std::vector<Signature> entityComponentSignatures;

		//takes a key type and maps it to a system instance
		std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

		//set of entities that are flagged to be added or removed from systems at next registry update
		std::set<Entity> entitiesToBeAdded;
		std::set<Entity> entitiesToBeKilled;

		//Entity tags (one tag name per entity)
		std::unordered_map<std::string, Entity> entityPerTag;
		std::unordered_map<int, std::string> tagPerEntity;

		//Entity groups (multiple groups per entity)
		std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
		std::unordered_map<int, std::string> groupPerEntity;

		//List of free ids that were removed
		std::deque<int> freeIds;


	public:
		Registry() { Logger::Log("Registry Constructor"); }
		~Registry() { Logger::Log("Registry Destructor"); }

		//the registry Update() finally process entietes that are wainting to be added or removed from systems
		void Update();

		//entity mangment
		Entity CreateEntity();
		void KillEntity(Entity entity);

		//adding componets to our pools and updating entity signatures
		template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
		template <typename TComponent> void RemoveComponent(Entity entity);
		template <typename TComponent> bool HasComponent(Entity entity);
		template <typename TComponent> TComponent& GetComponent(Entity entity) const;



		//System mangment section
		template <typename TSystem, typename ...TArgs> void AddSystem(TArgs ...args);
		template <typename TSystem> void RemoveSystem();
		template <typename TSystem> bool HasSystem() const;
		template <typename TSystem> TSystem& GetSystem() const;

		//tags management
		void TagEntity(Entity entity, const std::string& tag);
		bool EntityHasTag(Entity entity, const std::string& tag) const;
		Entity GetEntityByTag(const std::string& tag) const;
		void RemoveEntityTag(Entity entity);

		//groups management
		void GroupEntity(Entity entity, const std::string& group);
		bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
		std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
		void RemoveEntityGroup(Entity entity);

		//Add and remove entites from the system
		void AddEntityToSystems(Entity entity);
		void RemoveEntityFromSystems(Entity entity);
};









/// ///////////TComponent Management ///////////////

template <typename TComponent>
void System::RequireComponent() 
{
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	if(componentId >= componentPools.size())
	{
		componentPools.resize(componentId + 1, nullptr);
	}

	if(!componentPools[componentId])
	{
		std::shared_ptr <Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentId] = newComponentPool;
	}

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

	TComponent newComponent(std::forward<TArgs>(args)...);

	componentPool->Set(entityId, newComponent);

	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent> 
void Registry::RemoveComponent(Entity entity)
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	

	//remove the component from the component list for that entity
	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	componentPool->Remove(entityId);

	//Set the component bit to false in the entity signature
	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id = " + std::to_string(componentId) + " was removed from component entity id " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity)
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	return componentPool->Get(entityId);
}


///////////////System Management ///////////////

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs ...args)
{
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem> 
void Registry::RemoveSystem()
{
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

template <typename TSystem> 
bool Registry::HasSystem() const
{
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
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
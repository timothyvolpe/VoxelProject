#pragma once
#include <vector>
#include <limits>
#include <queue>
#include <array>
#include <unordered_map>
#include <memory>
#include "entity.h"
#include "componentdef.h"

class CGame;

//////////////
// Entities //
//////////////

/**
* @brief The entity manager class.
* @details This class maintains a set of entities identified by unique IDs. See https://austinmorlan.com/posts/entity_component_system/
*
* @author Timothy Volpe
* @date 4/26/2020
*/
class CEntityManager
{
private:
	EntityInt m_idRangeStart;

	std::queue<Entity> m_availableIds;
	std::array<ComponentSignature, ENTITY_MAX> m_entitySignatures;

	EntityInt m_activeEntities;

	/** Calculate the entities index from its ID by applying a simple offset related to m_idRangeStart */
	inline EntityInt calculateEntityIndex( Entity entity ) { return entity - m_idRangeStart; }
public:
	/**
	* @brief Constructor
	* @param[in]	idRangeStart	The first legal ID that this entity manager can create
	* @param[in]	idRangeStop		The last legal IDD that this entity manager can create. Only used for error checking, the last ID creatable will be idRangeStart+#ENTITY_MAX. Non-inclusive
	*/
	CEntityManager( EntityInt idRangeStart, EntityInt idRangeStop );

	/**
	* @brief Create a new entity by retrieving an available ID
	* @details This will attempt to retrieve an unused ID from the queue of available IDs. If one is available,
	*	it will be assigned to pEntity. If one is not available, false will be returned and pEntity will be untouched.
	*	A signature must be provided, as an empty signature will result in an invalid entity.
	* @param[in]	signature	The signature to assign to the new entity, must not be empty.
	* @param[out]	pEntity		The new entity ID still be stored here.
	* @returns Returns true if a new entity was available, false if #ENTITY_MAX was hit or the signature was invalid.
	*/
	bool CreateEntity( ComponentSignature signature, Entity *pEntity );

	/**
	* @brief Deletes an entity by adding its ID back to the queue of available IDs.
	* @details If the entity passed did not have a valid signature, it is considered to not exist and this function will return false.
	*	However if the entity pointed to a valid signature its ID will be returned to the available list and the signature reset.
	*/
	bool DestroyEntity( Entity entity );

	/**
	* @brief Sets an entities signature to a new value.
	* @details Cannot be an empty signature.
	* @param[in]	entity		The entity who's signature to set. If this entity has an invalid signature, the function will return false.
	* @param[in]	signature	The signature to set to the given entity.
	* @returns Returns true if successfully set valid signature, or false if the signature was invalid, or the entity was deem to not exist because its current signature was invalid.
	*/
	bool SetSignature( Entity entity, ComponentSignature signature );

	/**
	* @brief Retrieves an entities signature.
	* @details If the entity has an invalid signature, then the invalid signature will still be returned (none).
	* @param[in]	entity	The entities whose signature to return
	* @returns Returns the signature of the given entity.
	*/
	inline ComponentSignature GetSignature( Entity entity ) {
		assert( entity >= m_idRangeStart && entity < ENTITY_MAX+m_idRangeStart );
		return m_entitySignatures[this->calculateEntityIndex( entity )];
	}
};

////////////////
// Components //
////////////////

/**
* @brief The CComponentArray typeless interface.
*/
class IComponentArray
{
public:
	virtual bool AddEmptyComponent( Entity entity ) = 0;

	virtual void DestroyEntitiesComponent( Entity entity ) = 0;
};

/**
* @brief An array of entity components.
* @details This class maintains and array of a specific type of component, associated with entity IDs.
*	See https://austinmorlan.com/posts/entity_component_system/
*
* @author Timothy Volpe
* @date 4/26/2020
*/
template<class T>
class CComponentArray : public IComponentArray
{
private:
	std::array<T, ENTITY_MAX> m_componentArray;

	EntityInt m_activeComponents;

	std::unordered_map<Entity, size_t> m_entityToIndexMap;
	std::unordered_map<size_t, Entity> m_indexToEntityMap;
public:
	CComponentArray() {
		m_activeComponents = 0;
	}

	/**
	* @brief Add a component associated with an entity.
	* @details If there is no room left in the component array, this will fail.
	* @param[in]	entity		The entity to associate with the component.
	* @param[in]	component	The component to associate with the entity.
	* @returns Returns true if the component as added, or false if the component array count exceeded #ENTITY_MAX
	*/
	bool InsertComponent( Entity entity, T component )
	{
		assert( m_activeComponents < ENTITY_MAX );
		assert( m_entityToIndexMap.find( entity ) == m_entityToIndexMap.end() );

		if( m_activeComponents >= ENTITY_MAX )
			return false;

		// Put at the end of the component array
		size_t componentIndex = m_activeComponents;
		m_entityToIndexMap[entity] = componentIndex;
		m_indexToEntityMap[componentIndex] = entity;
		m_componentArray[componentIndex] = component;

		m_activeComponents++;

		return true;
	}

	/**
	* @brief Adds a blank component for a given entity
	* @param[in]	entity	The entity to add the component for
	* @returns See CComponentArray::InsertComponent
	*/
	bool AddEmptyComponent( Entity entity )
	{
		return this->InsertComponent( entity, T{} );
	}

	/**
	* @brief Remove an entities component
	* @details Swaps the last element in the array to the spot of the deleted entity to maintain contiguous data
	* @param[in]	entity	The entity whose component to remove
	*/
	void RemoveComponent( Entity entity )
	{
		assert( m_entityToIndexMap.find( entity ) != m_entityToIndexMap.end() );

		// Move entity from end into delete entities spot to maintain contiguous data
		size_t componentIndex = m_entityToIndexMap[entity];
		m_componentArray[componentIndex] = m_componentArray[m_activeComponents-1];
		// Set last entity to point to deleted entity's old spot
		m_entityToIndexMap[m_indexToEntityMap[m_activeComponents-1]] = componentIndex;
		m_indexToEntityMap[componentIndex] = m_indexToEntityMap[m_activeComponents-1];
		// Delete last element
		m_entityToIndexMap.erase( entity );
		m_indexToEntityMap.erase( m_activeComponents-1 );

		m_activeComponents--;
	}

	/**
	* @brief Get the value of a component associated with the given entity
	* @param[in]	entity		The entity whose component to retrieve
	* @param[out]	pComponent	The component associated with the entity is stored here.
	*/
	void GetComponent( Entity entity, T* pComponent )
	{
		assert( m_entityToIndexMap.find( entity ) != m_entityToIndexMap.end() );

		(*pComponent) = entity;

		return true;
	}

	/**
	* @brief Destroy an entities data in the component array
	*/
	void DestroyEntitiesComponent( Entity entity )
	{
		this->RemoveComponent( entity );
	}
};

/**
* @brief The component manager.
* @details This class manages a collection of components identified by component IDs
*	See https://austinmorlan.com/posts/entity_component_system/
*
* @author Timothy Volpe
* @date 4/26/2020
*/
class CComponentManager
{
private:
	std::unordered_map<const char*, ComponentType> m_componentTypes;
	std::unordered_map<ComponentType, const char*> m_componentTypeNameMap;
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_componentArrays;

	ComponentType m_activeComponentTypes;

	/** Retrieves a pointer to a component array of the given type, if registered. */
	template<typename T>
	std::shared_ptr<CComponentArray<T>> GetComponentArray()
	{
		const char* typeName = typeid(T).name();

		assert( m_componentTypes.find( typeName ) != m_componentTypes.end() );

		return std::static_pointer_cast<CComponentArray<T>>( m_componentArrays[typeName] );
	}
public:
	CComponentManager() {
		m_activeComponentTypes = 0;
	}

	/**
	* @brief Registers a component type with the manager.
	* @details The component type is identified by a string container its type identifier.
	*	If the maximum component types has been reached, set by #COMPONENT_TYPE_MAX, or the component has already been registered, the registration will fail.
	* @returns True if the component was registered, or false if #COMPONENT_TYPE_MAX has been reached, or the component has already been registered.
	*/
	template<typename T>
	bool RegisterComponent()
	{
		const char* typeName = typeid(T).name();

		assert( m_componentTypes.find( typeName ) == m_componentTypes.end() );
		assert( m_activeComponentTypes < COMPONENT_TYPE_MAX );

		if( m_componentTypes.find( typeName ) != m_componentTypes.end() )
			return false;
		if( m_activeComponentTypes >= COMPONENT_TYPE_MAX )
			return true;

		// Add to the component type map identifier
		m_componentTypes.insert( std::pair<const char*, ComponentType>( typeName, m_activeComponentTypes ) );
		m_componentTypeNameMap.insert( std::pair<ComponentType, const char*>( m_activeComponentTypes, typeName ) );
		m_componentArrays.insert( std::pair<const char*, std::shared_ptr<IComponentArray>>( typeName, std::make_shared<CComponentArray<T>>() ) );
		m_activeComponentTypes++;

		return true;
	}

	/**
	* @brief Get the type id for the given component, used in signatures.
	* @details The type id identifies which bit in the signature to set to enable that component for the entity.
	* @returns Returns the type id.
	*/
	template<typename T>
	ComponentType GetComponentTypeId() {
		const char* typeName = typeid(T).name();
		assert( m_componentTypes.find( typeName ) != m_componentTypes.end() );
		return m_componentTypes[typeName];
	}

	/**
	* @brief Add a component to a component type.
	* @details See CComponentArray::InsertComponent for more information. 
	* @param[in]	entity		The entity whose component to add
	* @param[in]	component	The component to add to the entity
	* @returns True if successfully added component, or false otherwise.
	*/
	template<typename T>
	bool AddComponent( Entity entity, T component ) {
		return this->GetComponentArray<T>()->InsertComponent( entity, component );
	}

	/**
	* @brief Removes a component from a component type.
	* @details see CComponentArray:RemoveComponent for more information.
	* @param[in]	entity	The entity whose component to remove.
	*/
	template<typename T>
	void RemoveComponent( Entity entity ) {
		return this->GetComponentArray<T>()->RemoveComponent( entity );
	}

	/**
	* @brief Add empty components for each item in the signature for the given entity.
	* @param[in]	signature	The signature defining which components to add
	* @param[in]	entity		The entity to add the components for
	*/
	void AddDefaultComponents( ComponentSignature signature, Entity entity );

	/**
	* @brief Remove all the components of a given entity based on the signature.
	* @param[in]	signature	Defines which components to remove.
	* @param[in]	entity		The entity whose components to remove.
	*/
	void RemoveAllComponents( ComponentSignature signature, Entity entity );

	/**
	* @brief Called when an entity is destroyed.
	* @details Checks entity for component types and destroys the entities valid components.
	* @param[in]	signature	The signature of the destroyed entity, used to find components.
	* @param[in]	entity		The entity ID of the destroyed entity
	*/
	void EntityDestroy( ComponentSignature signature, Entity entity );
};

/////////////
// Systems //
/////////////

class CSystemBase
{
protected:
	CGame *m_pGameHandle;

	std::vector<Entity> m_entities;
public:
	CSystemBase();
	CSystemBase( CGame *pGameHandle );

	virtual bool initialize() = 0;
	virtual void shutdown() = 0;

	virtual bool update( float deltaT ) = 0;

	/**
	* @brief Add an entity to be update by the system
	* @param[in]	entity	The entity to add to the system
	*/
	void addEntity( Entity entity );
	/**
	* @brief Remove an entity to be updated by the system
	* @param[in]	entity	The entity to remove from the system
	*/
	void removeEntity( Entity entity );
};


/**
* @brief The system manager.
* @details This class manages a collection of systems that connect components and entities.
*	See https://austinmorlan.com/posts/entity_component_system/
*
* @author Timothy Volpe
* @date 4/27/2020
*/
class CSystemManager
{
private:
	CGame* m_pGameHandle;

	std::unordered_map<const char*, std::shared_ptr<CSystemBase>> m_systemArray;
	std::unordered_map<const char*, ComponentSignature> m_systemSignatures;
public:
	CSystemManager( CGame* pGameHandle );

	/**
	* @brief Registers a system with the manager.
	* @details If the system has already been registered, the registration will fail. A pointer to the system
	*	is returned by the function if successfully registered the system. The signature defines which entities the system will pay attention to.
	* @param[in]	signature	The signature of entities for the system to update.
	* @returns Returns a pointer to the system if the registration was successful. If it failed, it will return a null pointer.
	*/
	template<typename T>
	std::shared_ptr<T> RegisterSystem( ComponentSignature signature )
	{
		const char* typeName = typeid(T).name();

		assert( m_systemArray.find( typeName ) == m_systemArray.end() );
		assert( !signature.none() );

		if( m_systemArray.find( typeName ) != m_systemArray.end() )
			return 0;

		std::shared_ptr<T> system = std::make_shared<T>( m_pGameHandle );
		system->initialize();
		m_systemArray.insert( std::pair<const char*, std::shared_ptr<CSystemBase>>( typeName, system ) );
		m_systemSignatures.insert( std::pair<const char*, ComponentSignature>( typeName, signature ) );

		return system;
	}

	/**
	* @brief Add an entity to the approriate signatures, based on the givem signature.
	* @param[in]	signature	The signature used to identify which systems to add the entity to.
	* @param[in]	entity		The entity to add to the systems.
	*/
	void AddEntityToSystems( ComponentSignature signature, Entity entity );

	/**
	* @brief Remove an entity from all the appropriate systems, based on given signature.
	* @param[in]	signature	The signature used to identify which systems to remove the entity from.
	* @param[in]	entity		The entity to remove from the systems.
	*/
	void RemoveEntityFromAll( ComponentSignature signature, Entity entity );
};

/**
* @brief Manages a group of entity-component-system managers.
* @details This allows each manager to talk to itself. See https://austinmorlan.com/posts/entity_component_system/
*
* @author Timothy Volpe
* @date 4/27/2020
*/
class CECSCoordinator
{
private:
	CGame* m_pGameHandle;

	CEntityManager* m_pEntityManager;
	CComponentManager* m_pComponentManager;
	CSystemManager* m_pSystemManager;
public:
	CECSCoordinator( CGame* pGameHandle, EntityInt idRangeStart, EntityInt idRangeStop );
	~CECSCoordinator();

	inline CEntityManager* getEntityManager() { return m_pEntityManager; }
	inline CComponentManager* getComponentManager() { return m_pComponentManager; }
	inline CSystemManager* getSystemManager() { return m_pSystemManager; }

	/**
	* @brief Create an entity and its components, and register it with the required systems.
	* @details The entity ID will be allocated, its signature will defined which components are created for it,
	*	as well as which systems it will be registered to.
	* @param[in]	signature	The signature to assign to the entity. The 3D position bit will automatically be set.
	* @param[out]	pEntity		The created entities ID will be stored here. Will be 0 if there was a failure.
	*/
	void createEntity( ComponentSignature signature, Entity* pEntity );

	/**
	* @brief Remove an entity, freeing up its ID and components.
	* @details The entity ID will be made available again, its components will be deleted and it will be
	*	unregistered from its systems.
	*/
	void removeEntity( Entity entity );
};
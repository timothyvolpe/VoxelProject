#pragma once
#include <glm\glm.hpp>
#include <vector>
#include <limits>
#include <queue>
#include <bitset>
#include <array>
#include <unordered_map>
#include <memory>
#include "entity.h"

#define ENTITY_MAX 1024
#define COMPONENT_TYPE_MAX 16

typedef std::bitset<COMPONENT_TYPE_MAX> ComponentSignature;

//////////////
// Entities //
//////////////

/** Guaranteed to be able to hold the maximum ID an entity can have. */
typedef uint32_t EntityInt;
typedef EntityInt Entity;


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
	std::queue<Entity> m_availableIds;
	std::array<ComponentSignature, ENTITY_MAX> m_entitySignatures;

	EntityInt m_activeEntities;
public:
	CEntityManager();

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
		return m_entitySignatures[entity];
	}
};

////////////////
// Components //
////////////////

typedef glm::vec3 Position3D;
typedef glm::vec3 Rotation3D;
typedef glm::vec3 Scale3D;

typedef uint16_t ComponentType;

struct Transform3DComponent
{
	Position3D	position;
	Rotation3D	rotation;
	Scale3D		scale;
};

/**
* @brief The CComponentArray typeless interface.
*/
class IComponentArray
{
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

	std::vector<T> m_componentArray;

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
		assert( m_activeComponents < <ENTITY_MAX );
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
		m_entityToIndexMap[mIndexToEntityMap[m_activeComponents-1]] = componentIndex;
		m_indexToEntityMap[componentIndex] = mIndexToEntityMap[m_activeComponents-1];
		// Delete last element
		m_entityToIndexMap.erase( entity );
		mIndexToEntityMap.erase( m_activeComponents-1 );

		m_activeComponents--;

		return true;
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
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_componentArrays;

	ComponentType m_activeComponentTypes;
public:
	CComponentManager() {
		m_activeComponentTypes = 0;
	}
	/**
	* @brief Registers a component type with the manager.
	* @details The component type is identified by a string container its type identifier.
	*	If the maximum component types has been reached, set by #COMPONENT_TYPE_MAX, the registration will fail.
	* @returns True if the component was registered, or false if #COMPONENT_TYPE_MAX has been reached.
	*/
	template<typename T>
	void RegisterComponent()
	{
		const char* typeName = typeid(T).name();

		assert( m_componentTypes.find( typeName ) == m_componentTypes.end() );
		assert( m_activeComponentTypes < COMPONENT_TYPE_MAX );

		if( m_activeComponentTypes >= COMPONENT_TYPE_MAX )
			return true;

		// Add to the component type map identifier
		m_componentTypes.insert( std::pair<const char*, ComponentType>( typeName, m_activeComponentTypes ) );
		m_componentTypes.insert( std::pair<const char*, std::shared_ptr<IComponentArray>( typeName, std::make_shared<CComponentArray<T>>() );
		m_activeComponentTypes++;
	}
};

/////////////
// Systems //
/////////////

class CEntitySystem
{
protected:
	std::vector<Entity> m_entities;
};

class CSystemManager
{

};
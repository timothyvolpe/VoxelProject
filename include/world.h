#pragma once
#include <memory>
#include "componentdef.h"

class CGame;
class CEntityManager;
class CComponentManager;
class CSystemManager;

class CRenderSystem;

/**
* @brief The world class which handles the 3D game world beyond the UI.
*
* @author Timothy Volpe
* @date 4/27/2020
*/
class CWorld
{
private:
	CGame* m_pGameHandle;

	CEntityManager* m_pWorldEntities;
	CComponentManager* m_pWorldComponents;
	CSystemManager* m_pWorldSystems;
public:
	CWorld( CGame* pGameHandle );
	~CWorld();

	/**
	* @brief Allocates and creates the game world
	*/
	bool createWorld();

	/**
	* @brief Cleanup world and free all memory
	*/
	void destroyWorld();

	/**
	* @brief Creates an entity in the world identified by an ID
	* @details This will register an entity with the entity manager and create its necessary components.
	* @param[in]	signature	The signature to assign to the entity. The 3D position bit will automatically be set.
	* @param[out]	pEntity		The created entities ID will be stored here.
	* @returns Returns true if the entity creation was successful, or false otherwise.
	*/
	bool createEntity( ComponentSignature signature, Entity *pEntity );

	/**
	* @brief Destroys an entity in the world identified by an ID
	* @details This will destroy an entity and its components and remove it from the world.
	* @param[in]	entity	The entity ID to delete
	*/
	void destroyEntity( Entity entity );

	/**
	* @brief Update all the entities in the world
	*/
	bool updateWorld( float deltaT );
};
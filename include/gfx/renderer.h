#pragma once
#include "componentdef.h"

class CGame;

class CEntityManager;
class CComponentManager;
class CSystemManager;

class CRenderSystem;

/**
* @brief The client-sided renderer, which renders 3D world objects.
* @details The world renderer handles networked entities from the server, as well as client-sided only entities.
*	All world entities have a 3D position.
*
* @author Timothy Volpe
* @date 12/10/2019
*/
class CWorldRenderer
{
private:
	CGame* m_pGameHandle;

	CEntityManager* m_pClientEntities;
	CComponentManager* m_pClientComponents;
	CSystemManager* m_pClientSystems;

	std::shared_ptr<CRenderSystem> m_renderSystem;
public:
	CWorldRenderer( CGame* pGameHandle );
	~CWorldRenderer();

	/**
	* @brief Initialize the client world renderer
	* @returns True if successful, false if otherwise
	*/
	bool initialize();

	/**
	* @brief Destroys the client sided world renderer and its data
	*/
	void destroy();

	/**
	* @brief Create an entity that is only available on the client
	* @details This creates and registers an entity only on the client.
	* @param[in]	signature	The signature to assign to the entity. The 3D position bit will automatically be set.
	* @param[out]	pEntity		The created entities ID will be stored here.
	*/
	bool createClientEntity( ComponentSignature signature, Entity *pEntity );
	/**
	* @brief Destroy an entity on the client
	* @details This destroys a client-sided entity. The entity must have been created as a client-sided entity
	*	in order to be destroyed.
	* @param[in]	entity	The entity ID to delete
	*/
	void destroyClientEntity( Entity entity );
};
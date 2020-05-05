#pragma once
#include "componentdef.h"

class CGame;

class CECSCoordinator;

class CRenderSystem;

/**
* @brief The client-sided renderer, which renders 3D world objects.
* @details The world renderer handles networked entities from the server, as well as client-sided only entities.
*	All world entities have a 3D position. This class also handles updating these entities but does mostly rendering.
*
* @author Timothy Volpe
* @date 12/10/2019
*/
class CWorldRenderer
{
private:
	CGame* m_pGameHandle;

	CECSCoordinator* m_pClientEntCoordinator;

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
	* @param[out]	pEntity		The created entities ID will be stored here. Will be 0 if there was a failure.
	*/
	void createClientEntity( ComponentSignature signature, Entity *pEntity );
	/**
	* @brief Destroy an entity on the client
	* @details This destroys a client-sided entity. The entity must have been created as a client-sided entity
	*	in order to be destroyed.
	* @param[in]	entity	The entity ID to delete
	*/
	void destroyClientEntity( Entity entity );

	/**
	* @brief Called during world loading
	* @details This is called after the ecs data has been set up, but before updates and rendering begin.
	*	This allows systems to act on all the entities initially assigned to them.
	* @returns True if successfully completed load operations, false if otherwise.
	*/
	bool onLoad();

	/**
	* @brief Update the client-sided entities but do not run the render system
	* @returns True if update was successful, false if otherwise
	*/
	bool update( float deltaT );
	/**
	* @brief Update the render system
	*/
	void render();
};
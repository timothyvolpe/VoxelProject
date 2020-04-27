/**
* @file client.h
* @brief Contains the CClient class, which handles all the client-only functionality
* @details Should be similar to #CServer but includes rendering
*
* @author Timothy Volpe
* @date 12/15/2019
*/

#pragma once
#include "componentdef.h"

class CGame;
class CGraphics;
class CConfig;
class CUserInput;
class CWorldRenderer;

/**
* @brief The client-sided handler.
* @details This class is in charge of everything that happens only on the client and is not shared
*	with the server. That includes rendering, sound, textures, etc.
*
* @author Timothy Volpe
* @date 12/15/2019
*/
class CClient
{
private:
	CGame *m_pGameHandle;

	CGraphics *m_pGraphics;
	CConfig *m_pClientConfig;
	CWorldRenderer* m_pWorldRenderer;

	CUserInput *m_pUserInput;

	Entity testEntity;

	void handleSDLEvents();

	bool loadConfig();
public:
	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	CClient( CGame *pGameHandle );
	/**
	* @brief Destructor.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	~CClient();

	/**
	* @brief Initializes the client class.
	* @return True if successfully initialized, false if otherwise. Error messagebox will be displayed,
	*		with associated log output.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	bool initialize();
	/**
	* @brief Destroys client data, and deletes pointers
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	void destroy();

	/**
	* @brief Client update function.
	* @details Handles updating all client entities and objects, should be run before rendering
	* @return Returns true if the update was successful, false otherwise.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	bool update();
	/**
	* @brief Client render function.
	* @details Handles render all the game entities, should be run after updating
	* @return Returns true if the render was successful, false otherwise.
	* @author Timothy Volpe
	* @date 12/12/2019
	*/
	bool render();

	/**
	* @brief Returns the main client config.
	* @return Client config.
	*/
	inline CConfig* CClient::getClientConfig() { return m_pClientConfig;  }
	

	/**
	* @brief Returns the user input handler class
	* @returns User input handler.
	*/
	inline CUserInput* CClient::getInputHandler() { return m_pUserInput;  }
};
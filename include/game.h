/**
* @file game.h
* @brief Defines the CGame class.
*
* @author Timothy Volpe
*
* @date 12/10/2019
*/

#pragma once

#include <chrono>

#define GAME_VERSION_MAJOR 0
#define GAME_VERSION_MINOR 1
#define GAME_VERSION_BUILD 2

class CLogger;
class CFilesystem;

class CClient;
class CServer;

/**
* @brief The main game class, everything starts and ends here.
* @details It should be created in the main function.
* It is not recommended to have more than one game class, as there will be conflicts accessing the 
* game files.
*
* @author Timothy Volpe
* @date 12/10/2019
*/
class CGame
{
private:
	bool m_bRunning;

	CLogger* m_pLogger;
	CFilesystem *m_pFilesystem;

	CClient *m_pClient;
	CServer *m_pServer;

	/** The frame time, recorded at the beginning of the frame */
	long long m_currentTimeUs;
	/** The time of the beginning of the last frame */
	long long m_lastFrameUs;
	/** The length of the last frame in seconds */
	double m_lastFrameTimeSeconds;
public:
	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	CGame();
	/**
	* @brief Destructor. 
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	~CGame();

	CGame( const CGame& ) = delete;

	/**
	* @brief Initializes the game class. Creates and initializes game objects.
	* @return True if successfully initialized, false if otherwise. Error messagebox will be displayed,
	*		with associated log output.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	bool initialize();
	/**
	* @brief Destroys game objects, and deletes pointers
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	void destroy();

	/**
	* @brief Starts the game loop
	* @details The game loop handles rendering everything, updating objects and entities, io, networking, everything.
	*	It should be called in the main function. This will block until the game is finished executing.
	* @return Returns true if the game exits normally, false if there was an error.
	* @author Timothy Volpe
	* @date 12/11/2019
	*/
	bool startGame();
	/**
	* @brief Finishes current frame then begins shutdown procedure
	* @details All this does is change the running flag to false, it does not interrupt any one-frame operations.
	*/
	void quitGame();

	/**
	* @brief Returns the logger object, which is owned by the game class.
	* @return Returns the logger object, which should not be deleted.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	CLogger* getLogger();
	/**
	* @brief Returns the filesystem object, which is owned by the game class.
	* @return Returns the filesystem object, which should not be deleted.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	CFilesystem* getFilesystem();
	/**
	* @brief Returns the client object, which is owned by the game class.
	* @return Returns the client object, which should not be deleted.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	CClient* getClient();

	/**
	* @brief Returns the last frame time in seconds
	* @details This is the time it took for the last frame to execute, recorded from the beginning of the last
	*	frame to the start of the current frame. It is constant throughout the entire active frame.
	*/
	double getFrameTime();
};
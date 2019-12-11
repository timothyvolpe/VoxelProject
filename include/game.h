/**
* @file game.h
* @brief Defines the CGame class.
*
* @author Timothy Volpe
*
* @date 12/10/2019
*/

#pragma once

class CLogger;
class CFilesystem;

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
	CLogger* m_pLogger;
	CFilesystem *m_pFilesystem;
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
	* @return True is successfully initialized, False if otherwise. Error messagebox will be displayed,
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
	* @brief Returns the logger object, which is owned by the game class.
	* @return Returns the logger object, which should not be deleted.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	CLogger* getLogger();
};
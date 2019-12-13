/**
* @file filesystem.h
* @brief Defines the CFilesystem class and contains general filesystem utilities.
*
* @author Timothy Volpe
*
* @date 12/11/2019
*/

#pragma once

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

/** Defines the location for the logger output */
#define FILEPATH_LOGFILE "console.log"

class CLogger;

/**
* @brief Filesystem handler class for the game
* @details This class handles the general filesystem tasks that the game needs to perform. That includes
*	making sure the necessary folder structure exists, converting relative paths to absolute ones, and
*	sanitizing user file input.
*
* @author Timothy Volpe
* @date 12/11/2019
*/
class CFilesystem
{
private:
	CLogger *m_pLoggerHandle;
public:
	/** List of directories that are required. Created in order, so parents should be before nested directories. */
	const std::vector<std::string> RequiredDirectories = { 
		"cache",
		"config",
		"config/server",
		"config/client",
		"textures",
		"sounds",
		"data",
		"data/shaders",
		"data/localization",
		"models"
	};

	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/11/2019
	*/
	CFilesystem( CLogger *pLogger );
	/**
	* @brief Destructor. Deletes all pointers.
	* @author Timothy Volpe
	* @date 12/11/2019
	*/
	~CFilesystem();

	/**
	* @brief Verifies the folder structure.
	* @details Examines the folder structure present in the games root directory to ensure that it is 
	*	valid, and creates any missing folders that are expected or required
	* @author Timothy Volpe
	* @date 12/11/2019
	*/
	bool verifyFilesystem();
};
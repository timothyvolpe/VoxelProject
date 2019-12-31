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
#include <map>
#include <boost/filesystem.hpp>

/** Defines the location for the logger output */
#define FILEPATH_LOGFILE "console.log"

class CLogger;

/**
* @brief Referencable locations in the filesystem
*/
enum FilesystemLocations
{
	LOCATION_CACHE,
	LOCATION_CONFIG,
	LOCATION_TEXTURES,
	LOCATION_SOUNDS,
	LOCATION_DATA,
	LOCATION_SHADERS,
	LOCATION_LOCALIZATION,
	LOCATION_MODELS
};

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
	const std::map<FilesystemLocations, std::string> RequiredDirectories = {
		{LOCATION_CACHE,		"cache"},
		{LOCATION_CONFIG,		"config"},
		{LOCATION_TEXTURES,		"textures"},
		{LOCATION_SOUNDS,		"sounds"},
		{LOCATION_DATA,			"data"},
		{LOCATION_SHADERS,		"data/shaders"},
		{LOCATION_LOCALIZATION,	"data/localization"},
		{LOCATION_MODELS,		"models"}
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

	/**
	* @brief Generates a usable filepath at a referencable location.
	* @details The filepath will be relative to the games working directory.
	* @param[in]	location		Represents the filesystem location the file should be found in
	* @param[in]	relativePath	The path relative to the filesystem location.
	* @return Returns a usable path relative to the working directory.
	*/
	boost::filesystem::path getGamePath( FilesystemLocations location, std::string relativePath );
};
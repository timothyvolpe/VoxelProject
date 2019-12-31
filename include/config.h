/**
* @file cpnfig.h
* @brief Contains the CConfig class.

* @author Timothy Volpe
* @date 12/30/2019
*/

#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>

/**
* @brief The config class for handling all the user config settings.
* @details This class handles saving and loading the config files, and exposing the user settings
* to the rest of the game. If a config file is not found, one is created with the hardcoded default values.
* There should be seperate configs for both client and server
*
* @author Timothy Volpe
* @date 12/30/2019
*/
class CConfig
{
public:
	/**
	* @brief Constructor.
	*/
	CConfig();
	/**
	* @brief Destructor.
	*/
	~CConfig();

	/**
	* @brief Loads a config file.
	* @details If the config file cannot be found, it must be created from the default values.
	* If it is found, it is checked to make sure all the expected values are present. Those that are missing
	* are created from the default values.
	* The config files should be in the .INI format.
	* @param[in]	configName			The name of the config file, relative to the config directory.
	* @param[out]	pConfigIndex		The config index is stored in this integer, which is used to reference the config data.
	* @param[in]	defaultProperties	A property tree containing the default config properties.
	* @return Returns true if the config file was successfully loaded/created, false if there was an unfixable error.
	*/
	bool loadConfig( std::string configName, unsigned int *pConfigIndex, boost::property_tree::ptree& defaultProperties );
	/**
	* @brief Saves a config file.
	* @details Saves the config data of the given index, to the file given when the config data was loaded.
	* @param[in]	configIndex	The config index referencing the config data.
	* @return Returns true if the config data was able to be saved to file, false if otherwise
	*/
	bool saveConfig( int configIndex );

	/**
	* @brief Saves all the config data.
	* @details Use this when shutting down to ensure all user data is preserved.
	*/
	void saveAll();
};

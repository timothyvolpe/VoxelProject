/**
* @file cpnfig.h
* @brief Contains the CConfig class.

* @author Timothy Volpe
* @date 12/30/2019
*/

#pragma once

#include <string>
#include <map>
#include <vector>
#include <boost/property_tree/ptree.hpp>

class CGame;

/**
* @brief The config class for handling user config settings.
* @details This class handles saving and loading a config file, and exposing the user settings
* to the rest of the game. If a config file is not found, one is created with the hardcoded default values.
* There should be seperate configs for both client and server
* Steps should be taken to minimize the amount of times properties are "retrived."
*
* @author Timothy Volpe
* @date 12/30/2019
*/
class CConfig
{
private:

	CGame *m_pGameHandle;

	std::string m_configName;
	boost::property_tree::ptree m_configProperties;
public:
	/**
	* @brief Constructor.
	*/
	CConfig( CGame *pGameHandle );
	/**
	* @brief Destructor.
	*/
	~CConfig();

	// TODO: Make CConfig only handle one config per class...

	/**
	* @brief Loads a config file.
	* @details If the config file cannot be found, it must be created from the default values.
	* If it is found, it is checked to make sure all the expected values are present. Those that are missing
	* are created from the default values.
	* The config files should be in the .INI format.
	* @param[in]	configName			The name of the config file, relative to the config directory.
	* @param[in]	defaultProperties	A property tree containing the default config properties.
	* @return Returns true if the config file was successfully loaded/created, false if there was an unfixable error.
	*/
	bool loadConfig( std::string configName, boost::property_tree::ptree& defaultProperties );
	/**
	* @brief Saves the config file.
	* @details Saves the config data to the file given when the config data was loaded.
	* @return Returns true if the config data was able to be saved to file, false if otherwise.
	*/
	bool saveConfig();

	/**
	* @brief Gets a property value from the config.
	* @details Finds the property at the path given.
	* If the property could not be found, then false will be returned and propertyValue will not be changed.
	* This function does not print a console message, so it is up to the caller to notify the user that there
	* has been an error.
	* @param[in]	propertyPath	The path to the property in the property tree
	* @param[out]	propertyValue	The value of the property. Only updated if the property was found.
	* @return Returns true if the property value was found, and false if it was not.
	*/
	template<class T>
	bool getPropertyFromConfig( std::string propertyPath, T* propertyValue )
	{
		auto prop = m_configProperties.get_optional<T>( propertyPath );
		if( prop )
		{
			(*propertyValue) = prop.get();
			return true;
		}
		else
			return false;
	}

	/**
	* @brief Updates a property in the config and saves to file.
	* @details If the given property exists, its value will be updated. If it does not, it will be created.
	* @param[in]	propertyPath	The path to the property in the property tree
	* @param[out]	propertyValue	The value of the property
	*/
	template<class T>
	void updateProperty( std::string propertyPath, T propertyValue )
	{
		m_configProperties.put<T>( propertyPath, propertyValue );
	}
};

#include <boost\property_tree\ini_parser.hpp>
#include "config.h"
#include "filesystem.h"
#include "game.h"
#include "logger.h"

CConfig::CConfig( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_configName = "";

}
CConfig::~CConfig() {
}

bool CConfig::loadConfig( std::string configName, boost::property_tree::ptree& defaultProperties )
{
	boost::filesystem::path configPath;
	boost::property_tree::ptree configTree;

	// Try to load the config file
	configPath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_CONFIG, configName );
	m_pGameHandle->getLogger()->print( "Loading config file \'%s\'", configName.c_str() );

	// if file exists, load it
	if( boost::filesystem::is_regular_file( configPath ) )
	{
		try {
			boost::property_tree::ini_parser::read_ini( configPath.string(), configTree );
		}
		catch( const boost::property_tree::ini_parser_error &e ) {
			m_pGameHandle->getLogger()->printError( "Failed to load config file \'%s\': %s", configName.c_str(), e.message().c_str() );
			return false;
		}

		// verify property tree 
		for( auto iter = defaultProperties.begin(); iter != defaultProperties.end(); iter++ )
		{
			auto checkprop = configTree.get_child_optional( (*iter).first );
			if( !checkprop )
				configTree.put_child( (*iter).first, (*iter).second );
		}
		m_configProperties = configTree;
	}
	// create it if it doesn't exist
	else
	{
		std::ofstream cfgFile( configPath.string(), std::ofstream::out );
		m_pGameHandle->getLogger()->printWarn( "Creating config file \'%s\'", configName.c_str() );
		if( !cfgFile ) {
			m_pGameHandle->getLogger()->printError( "Failed to create config file \'%s\'", configName.c_str() );
			return false;
		}
		cfgFile.close();

		// write default values
		boost::property_tree::ini_parser::write_ini( configPath.string(), defaultProperties );

		m_configProperties = defaultProperties;
	}

	m_configName = configName;

	return true;
}

bool CConfig::saveConfig()
{
	boost::filesystem::path configPath;

	configPath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_CONFIG, m_configName );
	try {
		boost::property_tree::ini_parser::write_ini( configPath.string(), m_configProperties );
	}
	catch( const boost::property_tree::ini_parser_error &e ) {
		m_pGameHandle->getLogger()->printError( "Failed to save config file \'%s\': %s", m_configName.c_str(), e.message().c_str() );
		return false;
	}
	return true;
}
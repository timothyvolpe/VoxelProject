#include "config.h"

CConfig::CConfig() {
}
CConfig::~CConfig() {
}

bool CConfig::loadConfig( std::string configName, unsigned int *pConfigIndex, boost::property_tree::ptree& defaultProperties )
{
	// Try to load the config file

	return true;
}

bool CConfig::saveConfig( int configIndex )
{
	return true;
}

void CConfig::saveAll()
{
}
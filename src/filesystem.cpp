#include "filesystem.h"
#include "logger.h"

CFilesystem::CFilesystem( CLogger *pLogger ) {
	m_pLoggerHandle = pLogger;
}
CFilesystem::~CFilesystem() {
}

bool CFilesystem::verifyFilesystem()
{
	m_pLoggerHandle->print( "Verifying folder structure..." );
	try
	{
		for( auto it = RequiredDirectories.begin(); it != RequiredDirectories.end(); it++ ) {
			if( !boost::filesystem::exists( it->second ) || !boost::filesystem::is_directory( it->second ) ) {
				m_pLoggerHandle->print( "Creating directory %s", it->second.c_str() );
				boost::filesystem::create_directory( it->second );
			}
		}
	}
	catch( const boost::filesystem::filesystem_error &e ) {
		m_pLoggerHandle->fatalMessageBox( "Failed to create missing game folders\n\n\'%s\'", e.what() );
		return false;
	}
	return true;
}

boost::filesystem::path CFilesystem::getGamePath( FilesystemLocations location, std::string relativePath )
{
	auto kvPair = RequiredDirectories.find( location );
	boost::filesystem::path usablePath;

	assert( kvPair != RequiredDirectories.end() );

	usablePath = kvPair->second;
	usablePath /= relativePath;

	return usablePath;
}
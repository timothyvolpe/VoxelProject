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
			if( !boost::filesystem::exists( (*it) ) || !boost::filesystem::is_directory( (*it) ) ) {
				m_pLoggerHandle->print( "Creating directory %s", (*it).c_str() );
				boost::filesystem::create_directory( (*it) );
			}
		}
	}
	catch( const boost::filesystem::filesystem_error &e ) {
		m_pLoggerHandle->fatalMessageBox( "Failed to create missing game folders\n\n\'%s\'", e.what() );
		return false;
	}
	return true;
}
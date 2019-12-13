#include "logger.h"
#include "filesystem.h"

CLogger::CLogger() {
	m_timeSinceLastFlush = 0;
}
CLogger::~CLogger() {
}

bool CLogger::flush()
{
	m_timeSinceLastFlush = 0;

	while( !m_logEntryQueue.empty() ) {
		std::string logEntry = m_logEntryQueue.front();
		if( m_logFile ) 
			m_logFile << logEntry << "\n";
		m_logEntryQueue.pop();
	}
	return true;
}

bool CLogger::start()
{
	// Open the log file
	m_logFile.open( FILEPATH_LOGFILE, std::ios::out | std::ios::trunc );
	if( !m_logFile ) {
		this->fatalMessageBox( "Failed to open the console.log file" );
		return false;
	}

	return true;
}
void CLogger::stop()
{
	this->flush();
	if( m_logFile )
		m_logFile.close();
}

bool CLogger::update( double elapsedTime )
{
	m_timeSinceLastFlush += elapsedTime;
	if( m_timeSinceLastFlush > FLUSH_INTERVAL )
		this->flush();

	return true;
}
#include "game.h"
#include "logger.h"
#include "filesystem.h"

CGame::CGame() {
	m_pLogger = 0;
	m_pFilesystem = 0;
}
CGame::~CGame() {
}

bool CGame::initialize()
{
	m_pLogger = new CLogger();
	if( !m_pLogger->start() )
		return false;

	m_pFilesystem = new CFilesystem();
	if( !m_pFilesystem->verifyFilesystem() )
		return false;

	return true;
}
void CGame::destroy()
{
	if( m_pLogger ) {
		m_pLogger->stop();
		delete m_pLogger;
		m_pLogger = 0;
	}
	if( m_pFilesystem ) {
		delete m_pFilesystem;
		m_pFilesystem = 0;
	}
}

CLogger* CGame::getLogger() {
	return m_pLogger;
}
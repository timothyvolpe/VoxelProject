/**
* @file game.cpp
* @brief Contains the CGame class
*
* @author Timothy Volpe
*
* @date 12/10/2019
*/

#include "game.h"
#include "logger.h"

CGame::CGame() {
	m_pLogger = 0;
}
CGame::~CGame() {
}

bool CGame::initialize()
{
	m_pLogger = new CLogger();
	if( !m_pLogger->start() )
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
}

CLogger* CGame::getLogger() {
	return m_pLogger;
}
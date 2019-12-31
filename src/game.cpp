#include "game.h"
#include "logger.h"
#include "filesystem.h"
#include "client.h"

CGame::CGame()
{
	m_bRunning = false;

	m_pLogger = 0;
	m_pFilesystem = 0;

	m_pClient = 0;

	m_currentTimeUs = 0;
	m_lastFrameUs = 0;
	m_lastFrameTimeSeconds = 0;
}
CGame::~CGame() {
}

bool CGame::initialize()
{
	m_pLogger = new CLogger();
	if( !m_pLogger->start() )
		return false;

	m_pLogger->print( "%s v%d.%d.%d", GAME_TITLE, GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_BUILD );

	m_pFilesystem = new CFilesystem( m_pLogger );
	if( !m_pFilesystem->verifyFilesystem() )
		return false;

	m_pClient = new CClient( this );
	if( !m_pClient->initialize() )
		return false;

	// so that the frame time isnt huge or 0
	auto curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
	m_lastFrameUs = std::chrono::duration_cast<std::chrono::microseconds>(curtime).count();

	return true;
}
void CGame::destroy()
{
	if( m_pClient ) {
		m_pClient->destroy();
		delete m_pClient;
		m_pClient = 0;
	}
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

bool CGame::startGame()
{
	m_bRunning = true;

	while( m_bRunning )
	{
		// Calculate time elapsed since last frame
		auto curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
		m_currentTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(curtime).count();
		m_lastFrameTimeSeconds = (m_currentTimeUs - m_lastFrameTimeSeconds) * 1000000;

		// Update client
		if( !m_pClient->update() )
			return false;
		// Update server...
		// Update logger last
		m_pLogger->update( this->getFrameTime() );

		// Render
		if( !m_pClient->render() )
			return false;
	}

	return true;
}

CLogger* CGame::getLogger() {
	return m_pLogger;
}
CFilesystem* CGame::getFilesystem() {
	return m_pFilesystem;
}
CClient* CGame::getClient() {
	return m_pClient;
}

double CGame::getFrameTime() {
	return m_lastFrameTimeSeconds;
}
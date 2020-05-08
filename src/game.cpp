#include "game.h"
#include "logger.h"
#include "filesystem.h"
#include "client.h"
#include "server.h"

CGame::CGame()
{
	m_bRunning = false;

	m_pLogger = 0;
	m_pFilesystem = 0;

	m_pClient = 0;
	m_pServer = 0;

	m_lastFrameTimeSeconds = 0;
}
CGame::~CGame() {
}

bool CGame::initialize()
{
	assert( !m_pLogger && !m_pClient && !m_pServer );

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

	m_pServer = new CServer( this );
	// Start server thread
	if( !m_pServer->initialize() )
		return false;
	if( !m_pServer->startServer() )
		return false;

	// so that the frame time isnt huge or 0
	m_lastFrame = std::chrono::high_resolution_clock::now();

	return true;
}
void CGame::destroy()
{
	// Shutdown server
	if( m_pServer ) {
		m_pServer->shutdownServer();
		delete m_pServer;
		m_pServer = 0;
	}
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
		m_currentTime = std::chrono::high_resolution_clock::now();
		m_lastFrameTimeSeconds = std::chrono::duration_cast<std::chrono::microseconds>(m_currentTime - m_lastFrame).count() / 1000000.0f;
		m_lastFrame = m_currentTime;

		// Update server
		if( !m_pServer->update() )
			return false;
		// Update client
		if( !m_pClient->update() )
			return false;

		// Update logger last
		m_pLogger->update( m_lastFrameTimeSeconds );

		// Render
		if( !m_pClient->render() )
			return false;
	}

	return true;
}
void CGame::quitGame() {
	this->m_bRunning = false;
	m_pLogger->print( "Quitting..." );
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

float CGame::getFrameTime() {
	return m_lastFrameTimeSeconds;
}
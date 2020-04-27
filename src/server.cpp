#include <chrono>
#include "server.h"
#include "game.h"
#include "logger.h"
#include "components.h"
#include "world.h"

CServer::CServer( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_serverRunning = false;
	m_pWorld = 0;

	testEntity = 0;
}
CServer::~CServer()
{
	if( m_serverRunning )
		this->shutdownServer();
}

void CServer::threadMain()
{
	m_pGameHandle->getLogger()->setServerThreadId( std::this_thread::get_id() );
	m_pGameHandle->getLogger()->print( "Starting server..." );

	// Notify main thread of server start
	std::unique_lock<std::mutex> lock( m_serverMutex );
	m_serverRunning = true;
	m_serverStartedFlag.notify_all();
	lock.unlock();

	m_currentTime = m_lastUpdate = std::chrono::high_resolution_clock::now();

	this->threadMainBody();

	// Notify the server has shutdown, should be at the end of the function
	m_pGameHandle->getLogger()->print( "Closing server..." );
	this->cleanupServer();

	lock.lock();
	m_serverRunning = false;
	m_serverStoppedFlag.notify_all();
	lock.unlock();
}

void CServer::threadMainBody()
{
	// Create the world
	m_pWorld = new CWorld( m_pGameHandle );
	if( !m_pWorld->createWorld() )
		return;

	// Create a test entity
	m_pWorld->createEntity( ComponentSignature(), &testEntity );
	if( !testEntity ) {
		m_pGameHandle->getLogger()->printError( "Failed to create test entity" );
		return;
	}

	// Server loop
	while( m_serverRunning )
	{
		if( !this->serverUpdate() )
			break;
	}
}

void CServer::cleanupServer()
{
	// Delete test entity
	if( testEntity ) {
		m_pWorld->destroyEntity( testEntity );
	}

	if( m_pWorld ) {
		m_pWorld->destroyWorld();
		delete m_pWorld;
		m_pWorld = 0;
	}
}

bool CServer::initialize()
{
	return true;
}

bool CServer::startServer()
{
	// Start the server thread
	m_serverThread = std::thread( &CServer::threadMain, this );

	// Wait for the server to signal it started or failed to start
	std::unique_lock<std::mutex> lock( m_serverMutex );
	m_serverStartedFlag.wait( lock );
	// Check if its still running
	if( !m_serverRunning ) {
		m_pGameHandle->getLogger()->printError( "The server failed to start!" );
		return false;
	}

	return true;
}
void CServer::shutdownServer()
{
	// Attempt to join the server thread
	if( m_serverRunning && m_serverThread.joinable() )
	{
		m_serverRunning = false;
		std::unique_lock<std::mutex> lock( m_serverMutex );
		// Wait SERVER_JOIN_TIMEOUT_MS milliseconds
		m_serverStoppedFlag.wait_until( lock, std::chrono::high_resolution_clock::now() + std::chrono::milliseconds( SERVER_JOIN_TIMEOUT_MS ) );
		m_serverThread.detach();
	}
	else
		m_serverRunning = false;
}

bool CServer::serverUpdate()
{
	m_currentTime = std::chrono::high_resolution_clock::now();
	m_lastUpdateTimeSeconds = std::chrono::duration_cast<std::chrono::microseconds>(m_currentTime - m_lastUpdate).count() * 1000000.0f;
	m_lastUpdate = m_currentTime;

	m_pWorld->updateWorld( m_lastUpdateTimeSeconds );

	return true;
}

bool CServer::update()
{
	

	if( !m_serverRunning )
		return false;

	return true;
}

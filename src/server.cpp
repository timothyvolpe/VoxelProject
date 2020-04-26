#include <chrono>
#include "server.h"
#include "game.h"
#include "logger.h"

CServer::CServer( CGame* pGameHandle ) : m_pGameHandle( pGameHandle ) {
	m_serverRunning = false;
}
CServer::~CServer() {
	if( m_serverRunning )
		this->shutdownServer();
}

void CServer::threadMain()
{
	// Inform logger
	m_pGameHandle->getLogger()->setServerThreadId( std::this_thread::get_id() );
	m_pGameHandle->getLogger()->print( "Starting server..." );

	// Notify main thread server start
	m_serverRunning = true;

	std::unique_lock<std::mutex> lock( m_serverMutex );
	m_serverStartedFlag.notify_all();
	lock.unlock();
	
	while( m_serverRunning ) {
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}

	// Notify the server has shutdown, should be at the end of the function
	m_pGameHandle->getLogger()->print( "Shutting down server..." );
	lock.lock();
	m_serverStoppedFlag.notify_all();
	lock.unlock();
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

bool CServer::update()
{
	return true;
}
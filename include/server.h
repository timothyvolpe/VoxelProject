#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

/** Amount of time in ms to wait for the server thread to join */
#define SERVER_JOIN_TIMEOUT_MS 5000

class CGame;

/**
* @brief The server-sided handler.
* @details This class is in charge of everything that happens only on the server and is not shared
*	with the client. That includes server sided entities, players, etc.

* @author Timothy Volpe
* @date 12/15/2019
*/
class CServer
{
private:
	CGame* m_pGameHandle;

	std::thread m_serverThread;

	std::mutex m_serverMutex;
	std::atomic<bool> m_serverRunning;
	std::condition_variable m_serverStartedFlag, m_serverStoppedFlag;

	void threadMain();
public:
	CServer( CGame* pGameHandle );
	~CServer();

	/**
	* @brief Setup the server but do not start the thread.
	* @returns Returns true if successful, false if otherwise
	*/
	bool initialize();

	/**
	* @brief Start the server thread and block until successfully started.
	* @details This is usually when the world is loaded, as the UI does not need the server to operate.
	* @returns Returns true if successfully started server, or false if otherwise.
	*/
	bool startServer();

	/**
	* @brief Shutdown the server and stop the server thread if it is running.
	* @details If the server thread is not running, it will be ignored. If it is, a join attempt will be made. If the attempt times out,
	*	the thread will be detached and the shutdown will continue.
	*/
	void shutdownServer();

	bool update();
};
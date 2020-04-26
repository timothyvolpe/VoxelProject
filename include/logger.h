/**
* @file logger.h
* @brief Defines the CLogger class.
*
* @author Timothy Volpe
*
* @date 12/10/2019
*/

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <boost\format.hpp>

#ifdef WIN32
#include <Windows.h>
#endif

#include "def.h"

/** Rough interval at which the log entries are flushed to the log file, in seconds */
#define FLUSH_INTERVAL 5.0

/**
* @brief Message logger class for debugging and general information.
* @details This class handles all the debugging and status information that needs to be given to the user.
* Output is sent to the console, as well as the log file. Output is dumped to the log file at a set interval, 
* or when the logger is shut down.
* Additionally, output can be displayed as a messagebox for more important notifications.
*
* @author Timothy Volpe
* @date 12/10/2019
*/
class CLogger
{
private:
	/**
	* @brief Describes the format that the logger will print to the console in.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	enum LOGGER_FORMAT
	{
		LOGGER_FORMAT_NORMAL,			/** Prints in white without any prefix */
		LOGGER_FORMAT_NORMAL_SERVER,	/** Prints in white with the [SERVER] prefix */
		LOGGER_FORMAT_WARNING,			/** Prints in yellow without any prefix, log file has prefix WARNING: */
		LOGGER_FORMAT_WARNING_SERVER,	/** Prints in yellow with the [SERVER] prefix, log file has prefix WARNING: */
		LOGGER_FORMAT_ERROR,			/** Prints in red without any prefix, log file has prefix ERROR: */
		LOGGER_FORMAT_ERROR_SERVER,		/** Prints in red with the [SERVER] prefix, log file has prefix ERROR: */
		LOGGER_FORMAT_LUA,				/** Prints in teal with the [LUA] prefix */
		LOGGER_FORMAT_LUA_SERVER,		/** Prints in teal with the [LUA][SERVER] prefix */
		LOGGER_FORMAT_LUA_ERROR,		/** Prints in red with the [LUA] prefix, log file has prefix ERROR: */
		LOGGER_FORMAT_LUA_ERROR_SERVER	/** Prints in red with the [LUA][SERVER] prefix, log file has prefix ERROR: */
	};

	std::queue<std::string> m_logEntryQueue;
	double m_timeSinceLastFlush;

	std::ofstream m_logFile;

	std::atomic<std::thread::id> m_serverThreadId;
	std::mutex m_logMutex;

	/**
	* @brief Print formatted helper function
	* Prints a formatted string to the console, as well as the log file.
	* Naturally, colors are not printed to the log file.
	* @param[in]	formatDesc	Enumerator specifying format.
	* @param[in]	formatStr	String describing the output format.
	* @param[in]	args		Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void printFormatted( LOGGER_FORMAT formatDesc, std::string formatStr, Args... args )
	{
		std::string prefix;
		std::string logPrefix, logStr;

		// prefix is appended to the console output and log file output
		// logPrefix is appended to the log file output, and should make up for the lack of color in the text file

		// Check if the function call is on the server thread
		if( std::this_thread::get_id() == m_serverThreadId ) {
			prefix = "[SERVER] ";
		}
		else
			prefix = "";
		logPrefix = "";

		switch( formatDesc )
		{
		case LOGGER_FORMAT_NORMAL:
			break;
		case LOGGER_FORMAT_WARNING:
			logPrefix += "WARNING: ";
			break;
		case LOGGER_FORMAT_ERROR:
			logPrefix += "ERROR: ";
			break;
		case LOGGER_FORMAT_LUA:
			prefix += "[LUA] ";
			break;
		case LOGGER_FORMAT_LUA_ERROR:
			prefix += "[LUA] ";
			logPrefix += "ERROR: ";
			break;
		}

		formatStr = prefix + formatStr;

		// use boost format
		boost::format formatter( formatStr );
		using unroll = int[]; unroll{ 0, (formatter % std::forward<Args>( args ), 0)... };

		// console print
		std::lock_guard<std::mutex> lock( m_logMutex );
		std::cout << boost::str( formatter ) << "\n";
		// log file print
		logStr = logPrefix + boost::str( formatter );
		m_logEntryQueue.push( logStr );
	}

	/**
	* @brief Flushes the log entries to the log file
	* @return Returns true if file write was successful, false otherwise
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	bool flush();
public:
	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	CLogger();
	/**
	* @brief Destructor. Deletes all pointers.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	~CLogger();

	/**
	* @brief Starts the logger. Attempts to open the log file.
	* @return True is successfully opened/created log file, false if otherwise. Error messagebox will be displayed,
	*		with associated log output.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	bool start();
	/**
	* @brief Flushes any unwritten log entries to the log file before closing it.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	void stop();

	/**
	* @brief Determines if it is neccessary to flush or not.
	* @details If the internal specified by #FLUSH_INTERVAL_MS has elapsed, the log entries are flushed to the file
	* @param[in]	elapsedTime	The time elapsed since the last update time, in seconds
	* @return Returns true if file write was successful, false otherwise
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	bool update( double elapsedTime );

	/**
	* @brief Tell the logger what the server thread's id is for identification
	* @details This id is used so the logger can determine when a print command is coming from the server
	*	instead of the client. This function is thread-safe.
	*/
	void setServerThreadId( std::thread::id threadId );

	/**
	* @brief Print and log informational message with newline character.
	* @details This function is thread-safe.
	* @param[in]	format	The output string format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void print( std::string format, Args... args ) {
		this->printFormatted( LOGGER_FORMAT_NORMAL, format, args... );
	}
	/**
	* @brief Print and log warning message with newline character.
	* @details This function is thread-safe.
	* @param[in]	format	The output string format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void printWarn( std::string format, Args... args ) {
		this->printFormatted( LOGGER_FORMAT_WARNING, format, args... );
	}
	/**
	* @brief Print and log error message with newline character.
	* @details This function is thread-safe.
	* @param[in]	format	The output string format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void printError( std::string format, Args... args ) {
		this->printFormatted( LOGGER_FORMAT_ERROR, format, args... );
	}

	/**
	* @brief Print and log lua informational message with newline character.
	* @details This function is thread-safe.
	* @param[in]	format	The output string format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void printLua( std::string format, Args... args ) {
		this->printFormatted( LOGGER_FORMAT_LUA, format, args... );
	}
	/**
	* @brief Print and log lua error message with newline character.
	* @details This function is thread-safe.
	* @param[in]	format	The output string format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void printLuaError( std::string format, Args... args ) {
		this->printFormatted( LOGGER_FORMAT_LUA_ERROR, format, args... );
	}

	/**
	* @brief Displays the appropriate fatal error message box.
	* @details This function is blocking, so it should only be used if the game is about to exit
	*	such as is in the case of a fatal error.
	* @param[in]	format	The message box contents format
	* @param[in]	args	Variable arguments to be inserted into formatStr
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	template<typename... Args>
	void fatalMessageBox( std::string format, Args... args )
	{
		// use boost format
		std::wstring wideFormat( format.begin(), format.end() );
		boost::wformat formatter( wideFormat );
		using unroll = int[]; unroll{ 0, (formatter % std::forward<Args>( args ), 0)... };
		std::string boxTitle = GAME_TITLE;
		std::wstring wideTitle( boxTitle.begin(), boxTitle.end() );

		// show platform dependant messagebox
#ifdef WIN32
		::MessageBox( NULL, boost::str( formatter ).c_str(), wideTitle.c_str(), MB_OK | MB_ICONERROR );
#endif
	}
};
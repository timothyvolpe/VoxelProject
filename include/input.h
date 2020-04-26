#pragma once
#include <SDL.h>
#include <queue>

/**
* @brief This class handles user input, namely from keyboard and mouse
* @details Keyboard presses are handled via the scancodes, so that the keys are dependent on their position on the keyboard,
*	not the value of the key. For user text input, keycodes must be used instead of scancodes so the input matches the keys pressed.
* @author Timothy Volpe
* @date 4/26/2020
*/
class CUserInput
{
private:
	/** Which scancodes are currently registered as pressed. */
	bool m_bScancodesPressed[SDL_NUM_SCANCODES];
	/** Which scancodes are currently registered as locked. */
	bool m_bScancodesLocked[SDL_NUM_SCANCODES];

	/** Which scancodes were pressed this frame and should be locked at the beginning of the next. */
	std::queue<unsigned int> m_scancodesToLock;
public:
	CUserInput();
	~CUserInput();

	/**
	* @brief Should be called near the beginning of the frame to detect new keypresses and lock old ones.
	*/
	void update();

	/**
	* @brief Signal that a key has been pressed, defined by an SDL event
	* @details Should be called from the SDL event loop when a SDL_KEYPRESS event is detected.
	*	Theoretically could be used to "spoof" keyboard events.
	*/
	void signalKeyDown( SDL_Event* pSdlEvent );
	/**
	* @brief Signal that a key has been release, defined by an SDL event
	* @details Should be called from the SDL event loop when a SDL_KEYUP event is detected.
	*	Theoretically could be used to "spoof" keyboard events.
	*/
	void signalKeyUp( SDL_Event* pSdlEvent );

	/**
	* @brief Check if a key was pressed this frame.
	* @details If they can is still down, but was not pressed this frame, this will return false.
	* @returns True if the key was pressed this frame, false otherwise.
	*/
	bool isKeyPressed( SDL_Scancode code );

	/**
	* @brief Check if a key is down.
	* @details Does not matter if it was pressed this frame or not.
	* @returns True if the key is down, false otherwise.
	*/
	bool isKeyHeld( SDL_Scancode code );
};

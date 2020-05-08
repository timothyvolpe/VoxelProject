#pragma once
#include <SDL.h>
#include <queue>

class CConfig;

#define DEFAULT_KEYBIND_FORWARD SDL_SCANCODE_W
#define DEFAULT_KEYBIND_BACKWARD SDL_SCANCODE_S
#define DEFAULT_KEYBIND_STRAFE_LEFT SDL_SCANCODE_A
#define DEFAULT_KEYBIND_STRAFE_RIGHT SDL_SCANCODE_D
#define DEFAULT_KEYBIND_RUN SDL_SCANCODE_LSHIFT
#define DEFAULT_KEYBIND_WALK SDL_SCANCODE_LCTRL

#define MAX_MOUSE_SENSITIVITY 200.0f
#define DEFAULT_MOUSE_SENSITIVITY 50.0f

/**
* @brief Maps keybinds to array indices so they can be easily referenced
*/
enum KeybindMap
{
	KEYBIND_INDEX_FORWARD = 0,
	KEYBIND_INDEX_BACKWARD,
	KEYBIND_INDEX_STRAFE_LEFT,
	KEYBIND_INDEX_STRAFE_RIGHT,
	KEYBIND_INDEX_RUN,
	KEYBIND_INDEX_WALK,
	KEYBIND_COUNT
};

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

	Sint32 m_lastMouseDX, m_lastMouseDY;

	/** Which scancodes were pressed this frame and should be locked at the beginning of the next. */
	std::queue<unsigned int> m_scancodesToLock;

	std::vector<SDL_Scancode> m_keybindScancodes;

	bool m_mouseInverted;
	float m_mouseSensitivity;

	/** Attempts to load keybind value from config, and if it doesnt exist or is invalid, uses default and updates config .*/
	SDL_Scancode loadKeybindValue( CConfig *pClientConfig, const char* pValue, int defaultValue );
public:
	CUserInput();
	~CUserInput();

	/**
	* @brief Loads and caches the keybinds from the client config
	* @details Missing keybinds are filled with their default values and the config is updated
	*/
	void loadKeybinds( CConfig *pClientConfig );

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
	* @brief Signal that the mouse has moved
	*/
	void signalMouseMove( SDL_Event *pSdlEvent );

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

	/**
	* @brief Same is isKeyPressed but for a particular keybind.
	* @param[in]	keybindIndex	The keybind to check for pressing.
	*/
	bool isKeybindPressed( KeybindMap keybindIndex );
	/**
	* @brief Same is isKeyHeld but for a particular keybind.
	* @param[in]	keybindIndex	The keybind to check for holding.
	*/
	bool isKeybindHeld( KeybindMap keybindIndex );

	/**
	* @brief Get the mouse delta X from last frame
	* @returns Mouse detla X from last frame
	*/
	inline const Sint32 getMouseDeltaX() { return m_lastMouseDX; }
	/**
	* @brief Get the mouse delta Y from last frame
	* @returns Mouse detla Y from last frame
	*/
	inline const Sint32 getMouseDeltaY() { return m_lastMouseDY; }

	/**
	* @brief Gets if the mouse is inverted
	*/
	inline bool isMouseInverted() { return m_mouseInverted; }
	/**
	* @brief Gets the mouse sensitivity
	*/
	inline const float getMouseSensitivity() { return m_mouseSensitivity; }
};

#include <assert.h>
#include <SDL.h>
#include "input.h"
#include "config.h"
#include "client.h"

CUserInput::CUserInput()
{
	memset( &m_bScancodesPressed, false, sizeof( m_bScancodesPressed ) );
	memset( &m_bScancodesLocked, false, sizeof( m_bScancodesLocked ) );

	m_lastMouseDX = 0;
	m_lastMouseDY = 0;

	m_mouseInverted = false;
	m_mouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
}
CUserInput::~CUserInput()
{
}

SDL_Scancode CUserInput::loadKeybindValue( CConfig *pClientConfig, const char* pValue, int defaultValue )
{
	int keybindVal;
	if( !pClientConfig->getPropertyFromConfig<int>( pValue, &keybindVal ) ) {
		pClientConfig->updateProperty( pValue, defaultValue );
		return (SDL_Scancode)defaultValue;
	}
	if( keybindVal >= SDL_NUM_SCANCODES || keybindVal < 0 ) {
		pClientConfig->updateProperty( pValue, defaultValue );
		return (SDL_Scancode)defaultValue;
	}
	return (SDL_Scancode)keybindVal;
}

void CUserInput::loadKeybinds( CConfig *pClientConfig )
{
	m_keybindScancodes.clear();
	m_keybindScancodes.resize( KeybindMap::KEYBIND_COUNT );

	if( !pClientConfig->getPropertyFromConfig<bool>( CONFIG_STR_MOUSE_INVERTED, &m_mouseInverted ) )
		m_mouseInverted = false;
	if( !pClientConfig->getPropertyFromConfig<float>( CONFIG_STR_MOUSE_SENSITIVITY, &m_mouseSensitivity ) )
		m_mouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
	if( m_mouseSensitivity <= 0 || m_mouseSensitivity > MAX_MOUSE_SENSITIVITY ) {
		m_mouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
		pClientConfig->updateProperty<float>( CONFIG_STR_MOUSE_SENSITIVITY, m_mouseSensitivity );
	}

	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_FORWARD] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_FORWARD, DEFAULT_KEYBIND_FORWARD );
	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_BACKWARD] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_BACKWARD, DEFAULT_KEYBIND_BACKWARD );
	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_STRAFE_LEFT] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_STRAFELEFT, DEFAULT_KEYBIND_STRAFE_LEFT );
	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_STRAFE_RIGHT] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_STRAFERIGHT, DEFAULT_KEYBIND_STRAFE_RIGHT );

	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_RUN] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_RUN, DEFAULT_KEYBIND_RUN );
	m_keybindScancodes[KeybindMap::KEYBIND_INDEX_WALK] = this->loadKeybindValue( pClientConfig, CONFIG_STR_KEYBOARD_WALK, DEFAULT_KEYBIND_WALK );
}

void CUserInput::update()
{
	// Clear mouse motion from last frame
	m_lastMouseDX = 0;
	m_lastMouseDY = 0;

	// Lock keys pressed last frame
	while( !m_scancodesToLock.empty() ) {
		// make sure its still pressed
		if( m_bScancodesPressed[m_scancodesToLock.front()] )
			m_bScancodesLocked[m_scancodesToLock.front()] = true;
		m_scancodesToLock.pop();
	}
}

void CUserInput::signalKeyDown( SDL_Event* pSdlEvent )
{
	assert( pSdlEvent->type == SDL_KEYDOWN );

	SDL_KeyboardEvent* pKeyboardEvent = reinterpret_cast<SDL_KeyboardEvent*>(pSdlEvent);

	// Set as pressed and register to lock if neccessary
	m_bScancodesPressed[pKeyboardEvent->keysym.scancode] = true;
	if( !m_bScancodesLocked[pKeyboardEvent->keysym.scancode] )
		m_scancodesToLock.push( pKeyboardEvent->keysym.scancode );
}
void CUserInput::signalKeyUp( SDL_Event* pSdlEvent )
{
	assert( pSdlEvent->type == SDL_KEYUP );

	SDL_KeyboardEvent* pKeyboardEvent = reinterpret_cast<SDL_KeyboardEvent*>(pSdlEvent);

	// Set as unpressed and unlocked
	m_bScancodesPressed[pKeyboardEvent->keysym.scancode] = false;
	m_bScancodesLocked[pKeyboardEvent->keysym.scancode] = false;
}
void CUserInput::signalMouseMove( SDL_Event *pSdlEvent )
{
	assert( pSdlEvent->type = SDL_MOUSEMOTION );

	SDL_MouseMotionEvent* pMouseEvent = reinterpret_cast<SDL_MouseMotionEvent*>(pSdlEvent);

	m_lastMouseDX = pMouseEvent->xrel;
	m_lastMouseDY = pMouseEvent->yrel;
}

bool CUserInput::isKeyPressed( SDL_Scancode code ) {
	assert( code < SDL_NUM_SCANCODES );
	return m_bScancodesPressed[code] && !m_bScancodesLocked[code];
}

bool CUserInput::isKeyHeld( SDL_Scancode code ) {
	assert( code < SDL_NUM_SCANCODES );
	return m_bScancodesPressed[code];
}

bool CUserInput::isKeybindPressed( KeybindMap keybindIndex ) {
	assert( keybindIndex < m_keybindScancodes.size() );
	return this->isKeyPressed( m_keybindScancodes[(int)keybindIndex] );
}

bool CUserInput::isKeybindHeld( KeybindMap keybindIndex ) {
	assert( keybindIndex < m_keybindScancodes.size() );
	return this->isKeyHeld( m_keybindScancodes[(int)keybindIndex] );
}
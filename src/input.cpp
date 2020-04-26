#include <assert.h>
#include "input.h"

CUserInput::CUserInput()
{
	memset( &m_bScancodesPressed, false, sizeof( m_bScancodesPressed ) );
	memset( &m_bScancodesLocked, false, sizeof( m_bScancodesLocked ) );
}
CUserInput::~CUserInput()
{
}

void CUserInput::update()
{
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

bool CUserInput::isKeyPressed( SDL_Scancode code ) {
	assert( code < SDL_NUM_SCANCODES );
	return m_bScancodesPressed[code] && !m_bScancodesLocked[code];
}

bool CUserInput::isKeyHeld( SDL_Scancode code ) {
	assert( code < SDL_NUM_SCANCODES );
	return m_bScancodesPressed[code];
}
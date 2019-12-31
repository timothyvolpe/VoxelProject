#include "gfx\graphics.h"
#include "game.h"
#include "logger.h"

int CGraphics::SDLReferenceCount = 0;

CGraphics::CGraphics( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pSDLWindow = 0;
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	m_pGameHandle->getLogger()->print( "Initializing graphics..." );

	// Initialize SDL2
	if( CGraphics::SDLReferenceCount <= 0 ) {
		m_pGameHandle->getLogger()->print( "Initializing SDL2 library..." );
		SDL_Init( SDL_INIT_VIDEO );
		CGraphics::SDLReferenceCount++;
	}
	else
		m_pGameHandle->getLogger()->print( "SDL2 already initialized" );

	// Create SDL window
	// TODO: Use window resolution from config
	m_pSDLWindow = SDL_CreateWindow(
		GAME_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_OPENGL
	);
	if( m_pSDLWindow == 0 ) {
		m_pGameHandle->getLogger()->print( "Failed to create SDL window\n\n%s", SDL_GetError() );
		return false;
	}

	return true;
}
void CGraphics::destroy()
{
	if( m_pSDLWindow ) {
		SDL_DestroyWindow( m_pSDLWindow );
		m_pSDLWindow = 0;
	}

	SDLReferenceCount--;
	if( SDLReferenceCount <= 0 ) {
		SDL_Quit();
	}
	m_pGameHandle = 0;
}
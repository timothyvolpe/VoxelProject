#include "gfx\graphics.h"
#include "game.h"
#include "logger.h"
#include "gfx\window.h"

CGraphics::CGraphics( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pWindow = 0;
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	m_pGameHandle->getLogger()->print( "Initializing graphics..." );

	// Initialize SDL2
	m_pGameHandle->getLogger()->print( "Initializing SDL2 library..." );
	SDL_Init( SDL_INIT_VIDEO );

	m_pWindow = new CWindow();
	if( !m_pWindow->initialize() )
		return false;

	return true;
}
void CGraphics::destroy()
{
	m_pGameHandle = 0;
}
#include <GL\glew.h>
#include <GL\GL.h>
#include "gfx\graphics.h"
#include "game.h"
#include "logger.h"
#include "config.h"
#include "client.h"

int CGraphics::SDLReferenceCount = 0;
bool CGraphics::GLEWInitialized = false;

CGraphics::CGraphics( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pSDLWindow = 0;
	m_sdlContext = 0;
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	SDL_version sdlVersion;
	unsigned int resX, resY;
	GLenum glewErr;

	m_pGameHandle->getLogger()->print( "Initializing graphics..." );

	// Initialize SDL2
	if( CGraphics::SDLReferenceCount <= 0 ) {
		m_pGameHandle->getLogger()->print( "Initializing SDL2 library..." );
		SDL_Init( SDL_INIT_VIDEO );
		SDL_GetVersion( &sdlVersion );
		m_pGameHandle->getLogger()->print( "Successfully initialized SDL v%d.%d.%d", (int)sdlVersion.major, (int)sdlVersion.minor, (int)sdlVersion.patch );
		CGraphics::SDLReferenceCount++;
	}
	else
		m_pGameHandle->getLogger()->print( "SDL2 already initialized" );

	// Get resolution
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( "ResolutionX", &resX );
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( "ResolutionY", &resY );

	// Create SDL window
	m_pSDLWindow = SDL_CreateWindow(
		GAME_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		resX,
		resY,
		SDL_WINDOW_OPENGL
	);
	if( m_pSDLWindow == 0 ) {
		m_pGameHandle->getLogger()->print( "Failed to create SDL window\n\n%s", SDL_GetError() );
		return false;
	}

	// Open GL attributes
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, OPENGL_RED_BITS );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, OPENGL_GREEN_BITS );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, OPENGL_BLUE_BITS );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, OPENGL_DEPTH_BITS );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR );

	m_pGameHandle->getLogger()->print( "Attempting to create OpenGL %d.%d context...", OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR );

	// Create the SDL context
	m_sdlContext = SDL_GL_CreateContext( m_pSDLWindow );
	if( !m_sdlContext ) {
		m_pGameHandle->getLogger()->print( "Failed to create OpenGL context: %s", SDL_GetError() );
		return false;
	}
	m_pGameHandle->getLogger()->print( "Sucessfully created OpenGL context!" );

	// Initialize GLEW
	m_pGameHandle->getLogger()->print( "Initializing GLEW..." );
	if( !CGraphics::GLEWInitialized )
	{
		glewErr = glewInit();
		if( glewErr != GLEW_OK ) {
			m_pGameHandle->getLogger()->print( "Failed it initialize GLEW: %s", glewGetErrorString( glewErr ) );
			return false;
		}
		CGraphics::GLEWInitialized = true;
	}
	m_pGameHandle->getLogger()->print( "Successfully initialize GLEW %s!", glewGetString( GLEW_VERSION ) );

	// Print GL info
	m_pGameHandle->getLogger()->print( "OpenGL Context Info:" );
	m_pGameHandle->getLogger()->print( "\tVersion: %s", glGetString( GL_VERSION ) );
	m_pGameHandle->getLogger()->print( "\tVendor: %s", glGetString( GL_VENDOR ) );
	m_pGameHandle->getLogger()->print( "\tRenderer: %s", glGetString( GL_RENDERER ) );
	m_pGameHandle->getLogger()->print( "\tGLSL: %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	return true;
}

bool CGraphics::draw()
{
	// Clear screen
	glClearColor( 0, 0, 0, 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	// Swap buffers
	SDL_GL_SwapWindow( m_pSDLWindow );

	return true;
}

void CGraphics::destroy()
{
	if( m_sdlContext ) {
		SDL_GL_DeleteContext( SDL_GL_DeleteContext );
		m_sdlContext = 0;
	}
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
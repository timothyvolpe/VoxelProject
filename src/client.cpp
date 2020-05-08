#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "client.h"
#include "game.h"
#include "config.h"
#include "input.h"
#include "logger.h"
#include "gfx\graphics.h"
#include "gfx\renderer.h"

CClient::CClient( CGame *pGameHandle )
{
	m_pGameHandle = pGameHandle;
	m_pGraphics = 0;
	m_pClientConfig = 0;
	m_pUserInput = 0;
	m_pWorldRenderer = 0;

	testEntity = 0;
}
CClient::~CClient() {
}

bool CClient::initialize()
{
	m_pClientConfig = new CConfig( m_pGameHandle );
	if( !this->loadConfig() )
		return false;

	m_pGraphics = new CGraphics( m_pGameHandle );
	if( !m_pGraphics->initialize() )
		return false;

	m_pWorldRenderer = new CWorldRenderer( m_pGameHandle );
	if( !m_pWorldRenderer->initialize() )
		return false;

	m_pUserInput = new CUserInput();
	m_pUserInput->loadKeybinds( m_pClientConfig );

	m_pWorldRenderer->createClientEntity( ComponentSignature(), &testEntity );
	if( !testEntity ) {
		m_pGameHandle->getLogger()->printError( "Failed to create test entity" );
		return false;
	}

	// Call on load
	if( !m_pWorldRenderer->onLoad() )
		return false;

	return true;
}
void CClient::destroy()
{
	if( testEntity ) {
		m_pWorldRenderer->destroyClientEntity( testEntity );
		testEntity = 0;
	}

	if( m_pUserInput ) {
		delete m_pUserInput;
		m_pUserInput = 0;
	}
	if( m_pWorldRenderer ) {
		m_pWorldRenderer->destroy();
		delete m_pWorldRenderer;
		m_pWorldRenderer = 0;
	}
	if( m_pGraphics ) {
		m_pGraphics->destroy();
		delete m_pGraphics;
		m_pGraphics = 0;
	}
	if( m_pClientConfig ) {
		m_pClientConfig->saveConfig();
		delete m_pClientConfig;
		m_pClientConfig = 0;
	}
	m_pGameHandle = 0;
}

bool CClient::loadConfig()
{
	boost::property_tree::ptree clientConfig;

	// Create default client config info
	clientConfig.put<unsigned int>( CONFIG_STR_RESOLUTION_X, DEFAULT_RESOLUTION_X );
	clientConfig.put<unsigned int>( CONFIG_STR_RESOLUTION_Y, DEFAULT_RESOLUTION_Y );
	clientConfig.put<unsigned int>( CONFIG_STR_REFRESH_RATE, DEFAULT_REFRESH_RATE );
	clientConfig.put<float>( CONFIG_STR_FOV, DEFAULT_FOV );
	clientConfig.put<int>( CONFIG_STR_WINDOW_MODE, DEFAULT_WINDOW_MODE );

	clientConfig.put<int>( CONFIG_STR_KEYBOARD_FORWARD, DEFAULT_KEYBIND_FORWARD );
	clientConfig.put<int>( CONFIG_STR_KEYBOARD_BACKWARD, DEFAULT_KEYBIND_BACKWARD );
	clientConfig.put<int>( CONFIG_STR_KEYBOARD_STRAFELEFT, DEFAULT_KEYBIND_STRAFE_LEFT );
	clientConfig.put<int>( CONFIG_STR_KEYBOARD_STRAFERIGHT, DEFAULT_KEYBIND_STRAFE_RIGHT );
	clientConfig.put<int>( CONFIG_STR_KEYBOARD_RUN, DEFAULT_KEYBIND_RUN );
	clientConfig.put<int>( CONFIG_STR_KEYBOARD_WALK, DEFAULT_KEYBIND_WALK );

	clientConfig.put<bool>( CONFIG_STR_MOUSE_INVERTED, false );
	clientConfig.put<float>( CONFIG_STR_MOUSE_SENSITIVITY, DEFAULT_MOUSE_SENSITIVITY );

	if( !m_pClientConfig->loadConfig( "client.cfg", clientConfig ) )
		return false;

	return true;
}

void CClient::handleSDLEvents()
{
	SDL_Event sdlEvent;

	while( SDL_PollEvent( &sdlEvent ) )
	{
		switch( sdlEvent.type )
		{
		case SDL_QUIT:
			m_pGameHandle->quitGame();
			break;
		case SDL_KEYDOWN:
			m_pUserInput->signalKeyDown( &sdlEvent );
			break;
		case SDL_KEYUP:
			m_pUserInput->signalKeyUp( &sdlEvent );
			break;
		case SDL_MOUSEMOTION:
			m_pUserInput->signalMouseMove( &sdlEvent );
			break;
		default:
			break;
		}
	}
}

bool CClient::update()
{
	// Update user input
	m_pUserInput->update();
	// SDL events
	this->handleSDLEvents();

	// Escape key quit for early testing
	if( m_pUserInput->isKeyPressed( SDL_SCANCODE_ESCAPE ) )
		m_pGameHandle->quitGame();

	// Update the client sided entities
	m_pWorldRenderer->update( m_pGameHandle->getFrameTime() );

	// Update graphics objects such as camera
	if( !m_pGraphics->update( m_pGameHandle->getFrameTime() ) )
		return false;

	return true;
}
bool CClient::render()
{
	// Render the client and networked entities
	m_pWorldRenderer->render();

	m_pGraphics->draw();

	return true;
}
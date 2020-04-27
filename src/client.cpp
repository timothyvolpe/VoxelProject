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

	if( !m_pWorldRenderer->createClientEntity( ComponentSignature(), &testEntity ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to create test entity" );
		return false;
	}

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
	if( !m_pWorldRenderer ) {
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
	clientConfig.put<int>( "ResolutionX", 1024 );
	clientConfig.put<int>( "ResolutionY", 728 );

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

	return true;
}
bool CClient::render()
{
	m_pGraphics->draw();

	return true;
}
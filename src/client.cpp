#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "client.h"
#include "game.h"
#include "config.h"
#include "gfx\graphics.h"
#include "gfx\window.h"

CClient::CClient( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pGraphics = 0;
	m_clientConfigIndex = 0;
}
CClient::~CClient() {
}

bool CClient::initialize()
{
	m_pConfig = new CConfig();
	if( !this->loadConfigs() )
		return false;

	m_pGraphics = new CGraphics( m_pGameHandle );
	if( !m_pGraphics->initialize() )
		return false;

	return true;
}
void CClient::destroy()
{
	if( m_pGraphics ) {
		m_pGraphics->destroy();
		delete m_pGraphics;
		m_pGraphics = 0;
	}
	if( m_pConfig ) {
		// TODO: Save
		delete m_pConfig;
		m_pConfig = 0;
	}
	m_pGameHandle = 0;
}

bool CClient::loadConfigs()
{
	boost::property_tree::ptree clientConfig;

	// Create default client config info
	clientConfig.put<int>( "ResolutionX", 1024 );
	clientConfig.put<int>( "ResolutionY", 728 );

	if( !m_pConfig->loadConfig( "client.cfg", &m_clientConfigIndex, clientConfig ) )
		return false;

	return true;
}

void CClient::handleSDLEvents()
{
	SDL_Event sdlEvent;

	while( SDL_PollEvent( &sdlEvent ) )
	{
		if( sdlEvent.type == SDL_QUIT )
			m_pGameHandle->quitGame();
	}
}

bool CClient::update()
{
	// SDL events
	this->handleSDLEvents();

	return true;
}
bool CClient::render()
{
	return true;
}
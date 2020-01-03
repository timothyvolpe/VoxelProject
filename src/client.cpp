#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "client.h"
#include "game.h"
#include "config.h"
#include "gfx\graphics.h"

CClient::CClient( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pGraphics = 0;
	m_pClientConfig = 0;
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

	return true;
}
void CClient::destroy()
{
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

CConfig* CClient::getClientConfig() {
	return m_pClientConfig;
}
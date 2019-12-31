#include "client.h"
#include "gfx\graphics.h"

CClient::CClient( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
	m_pGraphics = 0;
}
CClient::~CClient() {
}

bool CClient::initialize()
{
	m_pGraphics = new CGraphics( m_pGameHandle );
	if( !m_pGraphics->initialize() )
		return false;

	return true;
}
void CClient::destroy()
{
	m_pGameHandle = 0;
}

bool CClient::update()
{
	return true;
}
bool CClient::render()
{
	return true;
}
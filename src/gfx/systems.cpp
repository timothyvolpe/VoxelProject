#include "gfx/systems.h"

bool CRenderSystem::initialize()
{
	return true;
}
void CRenderSystem::shutdown()
{
}

bool CRenderSystem::update( float deltaT )
{
	for( auto it: m_entities )
	{

	}

	return true;
}
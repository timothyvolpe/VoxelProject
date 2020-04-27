#include "game.h"
#include "logger.h"
#include "components.h"
#include "gfx/renderer.h"
#include "gfx/systems.h"

CWorldRenderer::CWorldRenderer( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_renderSystem = 0;
}
CWorldRenderer::~CWorldRenderer()
{

}

bool CWorldRenderer::initialize()
{
	// Setup ECS stuff
	m_pClientEntCoordinator = new CECSCoordinator( m_pGameHandle, LOCAL_ID_RANGE_START, SHARED_ID_RANGE_START );

	m_pClientEntCoordinator->getComponentManager()->RegisterComponent<Position3DComponent>();
	m_pClientEntCoordinator->getComponentManager()->RegisterComponent<Transform3DComponent>();

	ComponentSignature renderSig;
	renderSig.set( m_pClientEntCoordinator->getComponentManager()->GetComponentTypeId<Position3D>() );
	m_renderSystem = m_pClientEntCoordinator->getSystemManager()->RegisterSystem<CRenderSystem>( renderSig );

	return true;
}
void CWorldRenderer::destroy()
{
	if( m_pClientEntCoordinator ) {
		delete m_pClientEntCoordinator;
		m_pClientEntCoordinator = 0;
	}
}

void CWorldRenderer::createClientEntity( ComponentSignature signature, Entity *pEntity )
{
	// Add 3d position
	signature.set( m_pClientEntCoordinator->getComponentManager()->GetComponentTypeId<Position3D>() );

	m_pClientEntCoordinator->createEntity( signature, pEntity );
}

void CWorldRenderer::destroyClientEntity( Entity entity )
{
	m_pClientEntCoordinator->removeEntity( entity );
}

bool CWorldRenderer::update( float deltaT )
{
	return true;
}

void CWorldRenderer::render()
{
	m_renderSystem->update( 0 );
}
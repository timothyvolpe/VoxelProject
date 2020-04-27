#include "world.h"
#include "game.h"
#include "logger.h"
#include "components.h"
#include "gfx/systems.h"

CWorld::CWorld( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_pWorldEntCoordinator = 0;
}
CWorld::~CWorld()
{
}

bool CWorld::createWorld()
{
	m_pGameHandle->getLogger()->print( "Creating world..." );

	// Setup ECS stuff
	m_pWorldEntCoordinator = new CECSCoordinator( m_pGameHandle, SHARED_ID_RANGE_START, SHARED_ID_RANGE_STOP );

	m_pWorldEntCoordinator->getComponentManager()->RegisterComponent<Position3DComponent>();
	m_pWorldEntCoordinator->getComponentManager()->RegisterComponent<Transform3DComponent>();

	return true;
}

void CWorld::destroyWorld()
{
	m_pGameHandle->getLogger()->print( "Cleaning up world..." );

	if( m_pWorldEntCoordinator ) {
		delete m_pWorldEntCoordinator;
		m_pWorldEntCoordinator = 0;
	}
}

void CWorld::createEntity( ComponentSignature signature, Entity *pEntity )
{
	// Add 3d position
	signature.set( m_pWorldEntCoordinator->getComponentManager()->GetComponentTypeId<Position3D>() );

	m_pWorldEntCoordinator->createEntity( signature, pEntity );
}
void CWorld::destroyEntity( Entity entity )
{
	m_pWorldEntCoordinator->removeEntity( entity );
}

bool CWorld::updateWorld( float deltaT )
{
	return true;
}
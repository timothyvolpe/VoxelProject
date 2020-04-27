#include "world.h"
#include "game.h"
#include "logger.h"
#include "components.h"
#include "gfx/systems.h"

CWorld::CWorld( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_pWorldEntities = 0;
	m_pWorldComponents = 0;
	m_pWorldSystems = 0;
}
CWorld::~CWorld()
{
}

bool CWorld::createWorld()
{
	m_pGameHandle->getLogger()->print( "Creating world..." );

	// Setup ECS stuff
	m_pWorldEntities = new CEntityManager( SHARED_ID_RANGE_START, SHARED_ID_RANGE_STOP );

	m_pWorldComponents = new CComponentManager();
	m_pWorldComponents->RegisterComponent<Position3DComponent>();
	m_pWorldComponents->RegisterComponent<Transform3DComponent>();

	m_pWorldSystems = new CSystemManager();

	return true;
}

void CWorld::destroyWorld()
{
	m_pGameHandle->getLogger()->print( "Cleaning up world..." );

	if( m_pWorldSystems ) {
		delete m_pWorldSystems;
		m_pWorldSystems = 0;
	}
	if( m_pWorldComponents ) {
		delete m_pWorldComponents;
		m_pWorldComponents = 0;
	}
	if( m_pWorldEntities ) {
		delete m_pWorldEntities;
		m_pWorldEntities = 0;
	}
}

bool CWorld::createEntity( ComponentSignature signature, Entity *pEntity )
{
	Entity newEntity;

	// Set 3D transform bit
	signature.set( m_pWorldComponents->GetComponentTypeId<Position3DComponent>() );

	// Create the entity with the manager
	if( !m_pWorldEntities->CreateEntity( signature, &newEntity ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to create entity, either because maximum entities was reached or the signature was invalid." );
		return false;
	}

	Position3DComponent transform =glm::vec3( 0, 0, 0 );
	m_pWorldComponents->AddComponent<Position3DComponent>( newEntity, transform );

	(*pEntity) = newEntity;

	return true;
}
void CWorld::destroyEntity( Entity entity )
{
	ComponentSignature signature = m_pWorldEntities->GetSignature( entity );
	m_pWorldEntities->DestroyEntity( entity );
	//m_pWorldComponents->EntityDestroy( signature );
	m_pGameHandle->getLogger()->printWarn( "Fix entity destruction" );
}

bool CWorld::updateWorld( float deltaT )
{
	return true;
}
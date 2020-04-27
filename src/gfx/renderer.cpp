#include "game.h"
#include "logger.h"
#include "components.h"
#include "gfx/renderer.h"
#include "gfx/systems.h"

CWorldRenderer::CWorldRenderer( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_pClientEntities = 0;
	m_pClientComponents = 0;
	m_pClientSystems = 0;

	m_renderSystem = 0;
}
CWorldRenderer::~CWorldRenderer()
{

}

bool CWorldRenderer::initialize()
{
	// Setup ECS stuff
	m_pClientEntities = new CEntityManager( LOCAL_ID_RANGE_START, SHARED_ID_RANGE_START );

	m_pClientComponents = new CComponentManager();
	m_pClientComponents->RegisterComponent<Position3DComponent>();
	m_pClientComponents->RegisterComponent<Transform3DComponent>();

	m_pClientSystems = new CSystemManager();
	m_renderSystem = m_pClientSystems->RegisterSystem<CRenderSystem>();

	return true;
}
void CWorldRenderer::destroy()
{
	if( m_pClientSystems ) {
		delete m_pClientSystems;
		m_pClientSystems = 0;
	}
	if( m_pClientComponents ) {
		delete m_pClientComponents;
		m_pClientComponents = 0;
	}
	if( m_pClientEntities ) {
		delete m_pClientEntities;
		m_pClientEntities = 0;
	}
}

bool CWorldRenderer::createClientEntity( ComponentSignature signature, Entity *pEntity )
{
	Entity newEntity;

	// Set 3D transform bit
	signature.set( m_pClientComponents->GetComponentTypeId<Position3DComponent>() );

	// Create the entity with the manager
	if( !m_pClientEntities->CreateEntity( signature, &newEntity ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to create entity, either because maximum entities was reached or the signature was invalid." );
		return false;
	}

	Position3DComponent transform =glm::vec3( 0, 0, 0 );
	m_pClientComponents->AddComponent<Position3DComponent>( newEntity, transform );

	(*pEntity) = newEntity;

	return true;
}

void CWorldRenderer::destroyClientEntity( Entity entity )
{
	ComponentSignature signature = m_pClientEntities->GetSignature( entity );
	m_pClientEntities->DestroyEntity( entity );
	//m_pWorldComponents->EntityDestroy( signature );
	m_pGameHandle->getLogger()->printWarn( "Fix entity destruction" );
}
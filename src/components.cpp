#include <algorithm>
#include "game.h"
#include "logger.h"
#include "components.h"

////////////////////
// CEntityManager //
////////////////////

CEntityManager::CEntityManager( EntityInt idRangeStart, EntityInt idRangeStop )
{
	assert( idRangeStart+ENTITY_MAX < idRangeStop );

	m_idRangeStart = idRangeStart;

	// Add all available IDs
	for( Entity i = m_idRangeStart; i < m_idRangeStart+ENTITY_MAX; i++ )
		m_availableIds.push( i );
	m_activeEntities = 0;
}

bool CEntityManager::CreateEntity( ComponentSignature signature, Entity *pEntity )
{
	assert( pEntity );
	assert( !signature.none() );

	if( signature.none() )
		return false;

	// Get the entity ID
	if( m_availableIds.empty() )
		return false;
	(*pEntity) = m_availableIds.front();
	m_availableIds.pop();
	m_activeEntities++;
	m_entitySignatures[this->calculateEntityIndex(*pEntity)] = signature;

	return true;
}
bool CEntityManager::DestroyEntity( Entity entity )
{
	assert( entity >= m_idRangeStart && entity < ENTITY_MAX+m_idRangeStart );
	assert( m_availableIds.size() == (ENTITY_MAX - m_activeEntities) ); // catch duplicates or other weird stuff

	EntityInt entityIndex = this->calculateEntityIndex( entity );

	// Make sure a valid signature exists to signal a valid entity
	if( m_entitySignatures[entityIndex].none() )
		return false;

	// Clear entity signature and add ID back to list of available
	m_entitySignatures[entityIndex].reset();
	m_availableIds.push( entity );

	return true;
}

bool CEntityManager::SetSignature( Entity entity, ComponentSignature signature )
{
	assert( entity >= m_idRangeStart && entity < ENTITY_MAX+m_idRangeStart );

	EntityInt entityIndex = this->calculateEntityIndex( entity );

	assert( !m_entitySignatures[entityIndex].none() );
	assert( !signature.none() );

	if( m_entitySignatures[entityIndex].none() )
		return false;
	if( signature.none() )
		return false;

	m_entitySignatures[entityIndex] = signature;

	return true;
}

////////////////
// Components //
////////////////

void CComponentManager::AddDefaultComponents( ComponentSignature signature, Entity entity )
{
	for( ComponentType i = 0; i < COMPONENT_TYPE_MAX; i++ )
	{
		// Check if set
		if( signature[i] )
		{
			assert( m_componentTypeNameMap.find( i ) != m_componentTypeNameMap.end() );

			// Find in component type map
			const char *pTypeName = m_componentTypeNameMap[i];
			assert( m_componentArrays.find( pTypeName ) != m_componentArrays.end() );
			m_componentArrays[pTypeName]->AddEmptyComponent( entity );
		}
	}
}
void CComponentManager::RemoveAllComponents( ComponentSignature signature, Entity entity )
{
	for( ComponentType i = 0; i < COMPONENT_TYPE_MAX; i++ )
	{
		// Check if set
		if( signature[i] )
		{
			assert( m_componentTypeNameMap.find( i ) != m_componentTypeNameMap.end() );

			// Find in component type map
			const char *pTypeName = m_componentTypeNameMap[i];
			assert( m_componentArrays.find( pTypeName ) != m_componentArrays.end() );
			m_componentArrays[pTypeName]->DestroyEntitiesComponent( entity );
		}
	}
}

void CComponentManager::EntityDestroy( ComponentSignature signature, Entity entity )
{
	for( auto it: m_componentArrays )
	{
		it.second->DestroyEntitiesComponent( entity );
	}
}

/////////////
// Systems //
/////////////

CSystemBase::CSystemBase() {
	m_pGameHandle = 0;
}
CSystemBase::CSystemBase( CGame *pGameHandle, CECSCoordinator *pCoordinatorHandle ) {
	m_pGameHandle = pGameHandle;
	m_pCoordinatorHandle = pCoordinatorHandle;
}

void CSystemBase::addEntity( Entity entity )
{
	assert( m_entities.size() <= ENTITY_MAX );

	// Add to vector
	m_entities.push_back( entity );
}

void CSystemBase::removeEntity( Entity entity )
{
	assert( m_entities.size() <= ENTITY_MAX );
	// Remove from vector
	m_entities.erase( std::remove( m_entities.begin(), m_entities.end(), entity ), m_entities.end() );
}

CSystemManager::CSystemManager( CGame* pGameHandle, CECSCoordinator *pCoordinatorHandle ) {
	m_pGameHandle = pGameHandle;
	m_pCoordinatorHandle = pCoordinatorHandle;
}

void CSystemManager::AddEntityToSystems( ComponentSignature signature, Entity entity )
{
	for( auto it: m_systemSignatures ) {
		// Check if they share the system signature bits
		if( (signature & it.second) == it.second ) {
			m_systemArray[it.first]->addEntity( entity );
		}
	}
}
void CSystemManager::RemoveEntityFromAll( ComponentSignature signature, Entity entity )
{
	for( auto it: m_systemSignatures ) {
		// Check if they share the system signature bits
		if( (signature & it.second) == it.second ) {
			m_systemArray[it.first]->removeEntity( entity );
		}
	}
}

bool CSystemManager::onLoad()
{
	for( auto it: m_systemArray ) {
		if( !it.second->onLoad() )
			return false;
	}
	return true;
}

/////////////////
// Coordinator //
/////////////////

CECSCoordinator::CECSCoordinator( CGame* pGameHandle, EntityInt idRangeStart, EntityInt idRangeStop ) : m_pGameHandle( pGameHandle )
{
	m_pEntityManager = new CEntityManager( idRangeStart, idRangeStop );
	m_pComponentManager = new CComponentManager();
	m_pSystemManager = new CSystemManager( pGameHandle, this );
}
CECSCoordinator::~CECSCoordinator()
{
	if( m_pEntityManager )
		delete m_pEntityManager;
	if( m_pComponentManager )
		delete m_pComponentManager;
	if( m_pSystemManager )
		delete m_pSystemManager;
}

void CECSCoordinator::createEntity( ComponentSignature signature, Entity* pEntity )
{
	Entity newEntity;

	(*pEntity) = 0;

	// Allocate entity ID
	if( !m_pEntityManager->CreateEntity( signature, &newEntity ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to create entity either because the signature was invalid or max entities was reached." );
		return;
	}
	// Add appropriate components
	m_pComponentManager->AddDefaultComponents( signature, newEntity );
	// Register to the appropriate systems
	m_pSystemManager->AddEntityToSystems( signature, newEntity );

	(*pEntity) = newEntity;
}

void CECSCoordinator::removeEntity( Entity entity )
{
	// Get entity signature
	ComponentSignature signature = m_pEntityManager->GetSignature( entity );

	// Free entity ID
	m_pEntityManager->DestroyEntity( entity );
	// Delete appropriate components
	m_pComponentManager->RemoveAllComponents( signature, entity );
	// Remove from appropriate systems
	m_pSystemManager->RemoveEntityFromAll( signature, entity );
}

bool CECSCoordinator::onLoad() {
	return m_pSystemManager->onLoad();
}
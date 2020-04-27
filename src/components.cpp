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

/////////////
// Systems //
/////////////
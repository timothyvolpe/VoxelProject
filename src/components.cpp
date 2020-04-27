#include "components.h"

////////////////////
// CEntityManager //
////////////////////

CEntityManager::CEntityManager()
{
	// Add all available IDs
	for( Entity i = 0; i < ENTITY_MAX; i++ )
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
	m_entitySignatures[(*pEntity)] = signature;

	return true;
}
bool CEntityManager::DestroyEntity( Entity entity )
{
	assert( entity < ENTITY_MAX );
	assert( m_availableIds.size() == (ENTITY_MAX - m_activeEntities) ); // catch duplicates or other weird stuff

	// Make sure a valid signature exists to signal a valid entity
	if( m_entitySignatures[entity].none() )
		return false;

	// Clear entity signature and add ID back to list of available
	m_entitySignatures[entity].reset();
	m_availableIds.push( entity );

	return true;
}

bool CEntityManager::SetSignature( Entity entity, ComponentSignature signature )
{
	assert( !m_entitySignatures[entity].none() );
	assert( !signature.none() );

	if( m_entitySignatures[entity].none() )
		return false;
	if( signature.none() )
		return false;

	m_entitySignatures[entity] = signature;

	return true;
}
#include <glm/ext.hpp>
#include "gfx/systems.h"
#include "gfx/graphics.h"
#include "gfx/shader.h"
#include "gfx/camera.h"
#include "game.h"
#include "logger.h"
#include "client.h"

///////////////////
// CRenderSystem //
///////////////////

CRenderSystem::CRenderSystem( CGame *pGameHandle, CECSCoordinator *pCoordinator )
{
	m_pGameHandle = pGameHandle;
	m_pCoordinatorHandle = pCoordinator;

	m_modelMatUniformLoc = -1;

	m_vertexArray = 0;
	m_vertexBuffer = 0;

	m_testCamera = std::make_shared<CCamera>();
}
CRenderSystem::~CRenderSystem() {
	this->shutdown();
}

bool CRenderSystem::initialize()
{
	// Get shader index
	if( !m_pGameHandle->getClient()->getGraphics()->getShaderManager()->getProgramIndex( "simple", &m_simpleShaderIndex ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to get simple shader program for client renderer." );
		return false;
	}
	m_simpleProgram = m_pGameHandle->getClient()->getGraphics()->getShaderManager()->getProgramByIndex( m_simpleShaderIndex );
	// Get the mvpUniformLocation
	size_t modelMatIndex;
	if( !m_simpleProgram->getUniformIndex( "modelMatrix", &modelMatIndex ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to get modelMatrix uniform location" );
		return false;
	}
	m_modelMatUniformLoc = m_simpleProgram->getUniformLocation( modelMatIndex );
	// Subscribe to uniform updates
	m_simpleProgram->subscribeToUniformUpdate( std::bind( &CRenderSystem::updateShaderUniforms, this ) );

	// Create the vao
	m_vertexArray = std::make_shared<CVertexArray>( m_pGameHandle );
	m_vertexArray->create();

	// Update uniforms once
	float fovRad = glm::radians( 100.0f );
	m_modelMatrix = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ) );
	m_simpleProgram->requireUniformUpdate();

	m_pGameHandle->getClient()->getGraphics()->setActiveCamera( m_testCamera );

	return true;
}
void CRenderSystem::shutdown()
{
	// Destroy the vao
	if( m_vertexArray ) {
		m_vertexArray->destroy();
		m_vertexArray.reset();
	}
	if( m_vertexBuffer ) {
		m_vertexBuffer->destroy();
		m_vertexArray.reset();
	}
}

bool CRenderSystem::onLoad()
{
	size_t entityCount = m_entities.size();

	// Create quads for each entity
	m_vertices.reserve( (unsigned int)entityCount * 4 );
	for( auto it: m_entities )
	{
		Position3D pos = m_pCoordinatorHandle->getComponentManager()->GetComponent<Position3D>( it );
		m_pGameHandle->getLogger()->print( "Coordinates for entity %d: (%f, %f, %f)", it, pos.x, pos.y, pos.z );

		m_vertices.push_back( { glm::vec3( 1.f, -1.f, 0.0f ) + pos } );
		m_vertices.push_back( { glm::vec3( 1.f, 1.f, 0.0f ) + pos } );
		m_vertices.push_back( { glm::vec3( -1.f, 1.f, 0.0f ) + pos } );
		m_vertices.push_back( { glm::vec3( -1.f, -1.f, 0.0f ) + pos } );
	}

	// Create vertex buffer objects
	m_vertexBuffer = std::make_shared<CBufferObject>();
	m_vertexBuffer->create( sizeof( Vertex3D ) * m_vertices.size(), &m_vertices[0], 0, GL_STATIC_DRAW );
	m_vertexArray->addBuffer( m_vertexBuffer, GL_ARRAY_BUFFER );

	m_vertexArray->addVertexAttrib( 3, GL_FLOAT, sizeof( Vertex3D ), (GLvoid*)offsetof( Vertex3D, position ), GL_FALSE );

	m_vertexArray->flushBindsAndAttribs();

	return true;
}

bool CRenderSystem::update( float deltaT )
{
	m_vertexArray->bind();

	for( auto it: m_entities )
	{

	}

	m_pGameHandle->getClient()->getGraphics()->submitForDraw( m_vertexArray, m_simpleShaderIndex, (unsigned int)m_vertices.size() );

	return true;
}

void CRenderSystem::updateShaderUniforms()
{
	glUniformMatrix4fv( m_modelMatUniformLoc, 1, GL_FALSE, glm::value_ptr( m_modelMatrix ) );
}
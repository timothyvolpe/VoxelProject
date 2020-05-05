#include "gfx/systems.h"
#include "gfx/graphics.h"
#include "gfx/shader.h"
#include "game.h"
#include "logger.h"
#include "client.h"

CRenderSystem::CRenderSystem( CGame *pGameHandle, CECSCoordinator *pCoordinator )
{
	m_pGameHandle = pGameHandle;
	m_pCoordinatorHandle = pCoordinator;

	m_vertexCount = 0;
}
CRenderSystem::~CRenderSystem() {
	this->shutdown();
}

bool CRenderSystem::initialize()
{
	// Get shader index
	if( !m_pGameHandle->getClient()->getGraphics()->getShaderManager()->getProgramIndex( "simple", &m_simpleShaderIndex ) ) {
		m_pGameHandle->getLogger()->print( "Failed to get simple shader program for client renderer." );
		return false;
	}
	// Get the mvpUniformLocation
	size_t mvpMatIndex;
	if( !m_pGameHandle->getClient()->getGraphics()->getShaderManager()->getProgramByIndex( m_simpleShaderIndex )->getUniformIndex( "MVPMatrix", &mvpMatIndex ) ) {
		m_pGameHandle->getLogger()->print( "Failed to get MVPMatrix uniform location" );
		return false;
	}
	m_mvpUniformLocation = m_pGameHandle->getClient()->getGraphics()->getShaderManager()->getProgramByIndex( m_simpleShaderIndex )->getUniformLocation( mvpMatIndex );

	// Create the vao
	m_vertexArray = std::make_shared<CVertexArray>( m_pGameHandle );
	m_vertexArray->create();
	m_vertexBuffer = std::make_shared<CBufferObject>();

	m_vertexArray->addVertexAttrib( 3, GL_FLOAT, sizeof( glm::vec3 ), (void*)offsetof( Vertex3D, position ), GL_FALSE );

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
	std::vector<Vertex3D> vertices;

	// Create quads for each entity
	m_vertexCount = (unsigned int)entityCount * 4;
	vertices.reserve( m_vertexCount );
	for( auto it: m_entities )
	{
		Position3D pos = m_pCoordinatorHandle->getComponentManager()->GetComponent<Position3D>( it );
		//m_pGameHandle->getLogger()->print( "Coordinates for entity %d: (%f, %f, %f)", it, pos.x, pos.y, pos.z );

		vertices.push_back( { glm::vec3( 0.5f, -0.5f, 0.0f ) + pos } );
		vertices.push_back( { glm::vec3( 0.5f, 0.5f, 0.0f ) + pos } );
		vertices.push_back( { glm::vec3( -0.5f, 0.5f, 0.0f ) + pos } );
		vertices.push_back( { glm::vec3( -0.5f, -0.5f, 0.0f ) + pos } );
	}

	// Create vertex buffer objects
	m_vertexBuffer->create( sizeof( vertices ), std::make_shared<Vertex3D>( vertices[0] ), 0, GL_STATIC_DRAW );
	m_vertexArray->addBuffer( m_vertexBuffer, GL_ARRAY_BUFFER );

	m_vertexArray->flushBindsAndAttribs();

	return true;
}

bool CRenderSystem::update( float deltaT )
{
	m_vertexArray->bind();

	for( auto it: m_entities )
	{

	}

	m_pGameHandle->getClient()->getGraphics()->submitForDraw( m_vertexArray, m_simpleShaderIndex, m_vertexCount );

	return true;
}
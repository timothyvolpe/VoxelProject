#include "gfx/systems.h"
#include "gfx/graphics.h"

CRenderSystem::CRenderSystem( CGame *pGameHandle ) {
	m_pGameHandle = pGameHandle;
}
CRenderSystem::~CRenderSystem() {
	this->shutdown();
}

bool CRenderSystem::initialize()
{
	// Create the vao
	m_vertexArray = std::make_unique<CVertexArray>( m_pGameHandle );
	m_vertexArray->create();
	m_vertexBuffer = std::make_shared<CBufferObject>();
	m_vertexBuffer->create();
	m_vertexArray->addBuffer( m_vertexBuffer, GL_ARRAY_BUFFER );

	m_vertexArray->flushBinds();

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

bool CRenderSystem::update( float deltaT )
{
	m_vertexArray->bind();

	for( auto it: m_entities )
	{

	}

	return true;
}
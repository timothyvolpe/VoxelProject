#include <algorithm>
#include "gfx\graphics.h"
#include "gfx\shader.h"
#include "game.h"
#include "logger.h"
#include "config.h"
#include "client.h"

int CGraphics::SDLReferenceCount = 0;
bool CGraphics::GLEWInitialized = false;

bool RenderJobSort( RenderJob& a, RenderJob& b ) {
	return a.shaderIndex < b.shaderIndex;
}

///////////////
// CGraphics //
///////////////

CGraphics::CGraphics( CGame *pGameHandle )
{
	m_pGameHandle = pGameHandle;
	m_glSupportLevel = GLSupportLevel::GL_SUPPORT_NONE;
	m_pSDLWindow = 0;
	m_sdlContext = 0;

	m_pShaderManager = 0;
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	SDL_version sdlVersion;
	unsigned int resX, resY;
	GLenum glewErr;
	GLenum glError;

	m_pGameHandle->getLogger()->print( "Initializing graphics..." );

	// Initialize SDL2
	if( CGraphics::SDLReferenceCount <= 0 ) {
		m_pGameHandle->getLogger()->print( "Initializing SDL2 library..." );
		SDL_Init( SDL_INIT_VIDEO );
		SDL_GetVersion( &sdlVersion );
		m_pGameHandle->getLogger()->print( "Successfully initialized SDL v%d.%d.%d", (int)sdlVersion.major, (int)sdlVersion.minor, (int)sdlVersion.patch );
		CGraphics::SDLReferenceCount++;
	}
	else
		m_pGameHandle->getLogger()->printWarn( "SDL2 already initialized" );

	// Get resolution
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( "ResolutionX", &resX );
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( "ResolutionY", &resY );

	// Create SDL window
	m_pSDLWindow = SDL_CreateWindow(
		GAME_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		resX,
		resY,
		SDL_WINDOW_OPENGL
	);
	if( m_pSDLWindow == 0 ) {
		m_pGameHandle->getLogger()->printError( "Failed to create SDL window\n\n%s", SDL_GetError() );
		return false;
	}

	// Open GL attributes
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, OPENGL_RED_BITS );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, OPENGL_GREEN_BITS );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, OPENGL_BLUE_BITS );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, OPENGL_DEPTH_BITS );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR );

	m_pGameHandle->getLogger()->print( "Attempting to create OpenGL %d.%d context...", OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR );

	// Create the SDL context
	m_sdlContext = SDL_GL_CreateContext( m_pSDLWindow );
	if( !m_sdlContext ) {
		m_pGameHandle->getLogger()->printError( "Failed to create OpenGL context: %s", SDL_GetError() );
		return false;
	}
	m_pGameHandle->getLogger()->print( "Sucessfully created OpenGL context!" );

	// Initialize GLEW
	m_pGameHandle->getLogger()->print( "Initializing GLEW..." );
	if( !CGraphics::GLEWInitialized )
	{
		glewErr = glewInit();
		if( glewErr != GLEW_OK ) {
			m_pGameHandle->getLogger()->printError( "Failed it initialize GLEW: %s", glewGetErrorString( glewErr ) );
			return false;
		}
		CGraphics::GLEWInitialized = true;
	}
	m_pGameHandle->getLogger()->print( "Successfully initialize GLEW %s", glewGetString( GLEW_VERSION ) );
	// Check for open GL support
	if( glewIsSupported( "GL_VERSION_3_2" ) )
		m_glSupportLevel = GLSupportLevel::GL_SUPPORT_MIN;
	if( glewIsSupported( "GL_VERSION_4_1" ) )
		m_glSupportLevel = GLSupportLevel::GL_SUPPORT_STD;
	if( glewIsSupported( "GL_VERSION_4_6" ) )
		m_glSupportLevel = GLSupportLevel::GL_SUPPORT_MAX;
	if( m_glSupportLevel == GLSupportLevel::GL_SUPPORT_NONE ) {
		m_pGameHandle->getLogger()->printError( "The minimum OpenGL supported version is 3.2, which this computer does not support!" );
		return false;
	}

	// Print GL info
	m_pGameHandle->getLogger()->print( "OpenGL Context Info:" );
	m_pGameHandle->getLogger()->print( "\tVersion: %s", glGetString( GL_VERSION ) );
	m_pGameHandle->getLogger()->print( "\tVendor: %s", glGetString( GL_VENDOR ) );
	m_pGameHandle->getLogger()->print( "\tRenderer: %s", glGetString( GL_RENDERER ) );
	m_pGameHandle->getLogger()->print( "\tGLSL: %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Check for error, however if we have gotten this far it is probably fine
	if( (glError = glGetError()) != GL_NO_ERROR )
		m_pGameHandle->getLogger()->printError( "There was a GL error during the initialization process, GL error code %u", glError );

	// Create shader manager
	m_pShaderManager = new CShaderManager( m_pGameHandle );
	if( !m_pShaderManager->initialize() )
		return false;
	// Load the shader files
	if( !m_pShaderManager->loadPrograms() )
		return false;

	return true;
}

bool CGraphics::draw()
{
	// Clear screen
	glClearColor( 0, 0, 0, 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	// Swap buffers
	SDL_GL_SwapWindow( m_pSDLWindow );

	// Sort submitted vertex arrays by shader id
	std::sort( m_renderJobs.begin(), m_renderJobs.end(), RenderJobSort );

	// Draw vertices
	for( auto it: m_renderJobs )
	{
		m_pShaderManager->bindProgram( it.shaderIndex );
		it.vertexArray->bind();
		glDrawArrays( GL_POINTS, 0, it.vertexCount );
	}

	// Reset to old size to maybe save some time
	size_t pastSize = m_renderJobs.size();
	m_renderJobs.clear();
	m_renderJobs.reserve( pastSize );

	return true;
}

void CGraphics::destroy()
{
	if( m_pShaderManager ) {
		m_pShaderManager->shutdown();
		delete m_pShaderManager;
		m_pShaderManager = 0;
	}
	if( m_sdlContext ) {
		SDL_GL_DeleteContext( SDL_GL_DeleteContext );
		m_sdlContext = 0;
	}
	if( m_pSDLWindow ) {
		SDL_DestroyWindow( m_pSDLWindow );
		m_pSDLWindow = 0;
	}

	SDLReferenceCount--;
	if( SDLReferenceCount <= 0 ) {
		SDL_Quit();
	}
	m_pGameHandle = 0;
}

void CGraphics::submitForDraw( std::shared_ptr<CVertexArray> vertexArray, unsigned int shaderIndex, unsigned int vertexCount )
{
	assert( vertexArray );
	assert( vertexArray->getVertexArrayId() );
	assert( shaderIndex );

	m_renderJobs.push_back( { vertexArray, shaderIndex, vertexCount } );
}

///////////////////
// CBufferObject //
///////////////////

CBufferObject::CBufferObject()
{
	m_bufferId = 0;

	m_bufferSize = 0;
	m_bufferFlags = 0;
	m_bufferUsage = 0;
}
CBufferObject::~CBufferObject()
{
	this->destroy();
}

void CBufferObject::create( GLsizeiptr size, std::shared_ptr<void> data, GLbitfield flags, GLenum usage )
{
	assert( !m_bufferId && !m_bufferData );

	m_bufferSize = size;
	m_bufferData = data;
	m_bufferFlags = flags;
	m_bufferUsage = usage;
}
void CBufferObject::destroy()
{
	if( m_bufferId ) {
		glDeleteBuffers( 1, &m_bufferId );
		m_bufferId = 0;
	}
}

bool CBufferObject::bind( GLenum target )
{
	GLenum glError;

	// if the buffer hasnt actually been created
	if( !m_bufferId )
	{
		glGenBuffers( 1, &m_bufferId );
		glBindBuffer( target, m_bufferId );
		if( glewIsSupported( "GL_ARB_buffer_storage" ) )
			glBufferStorage( target, m_bufferSize, m_bufferData.get(), m_bufferFlags );
		else
			glBufferData( target, m_bufferSize, m_bufferData.get(), m_bufferUsage );
		if( (glError = glGetError()) != GL_NO_ERROR ) {
			m_bufferId = 0;
			return false;
		}
	}
	else
		glBindBuffer( target, m_bufferId );

	return true;
}

//////////////////
// CVertexArray //
//////////////////

CVertexArray::CVertexArray( CGame* pGameHandle ) : m_pGameHandle( pGameHandle )
{
	m_vaoId = 0;
	m_vertexAttribsActive = 0;
}
CVertexArray::~CVertexArray()
{
	this->destroy();
}

bool CVertexArray::create()
{
	assert( !m_vaoId );

	GLenum glError;

	glGenVertexArrays( 1, &m_vaoId );
	if( (glError = glGetError()) != GL_NO_ERROR ) {
		m_vaoId = 0;
		m_pGameHandle->getLogger()->printError( "Failed to generate vertex array, GL error code %u", glError );
		return false;
	}
	return true;
}
void CVertexArray::destroy()
{
	if( m_vaoId ) {
		glDeleteVertexArrays( 1, &m_vaoId );
		m_vaoId = 0;
	}
	m_vertexAttribsActive = 0;
	m_buffersToBind.clear();
	m_vertexAttribQueue = std::queue<CVertexArray::VertexAttribPointer>();
}

void CVertexArray::bind()
{
	assert( m_vaoId );

	glBindVertexArray( m_vaoId );
}

void CVertexArray::addBuffer( std::shared_ptr<CBufferObject> bufferObject, GLenum target )
{
	assert( bufferObject );

	m_buffersToBind.push_back( BufferPair( target, bufferObject ) );
}

GLuint CVertexArray::addVertexAttribInternal( unsigned char internalType, GLint size, GLenum type, GLsizei stride, const void *pointer, GLboolean normalized )
{
	assert( m_vaoId );
	assert( m_vertexAttribsActive < GL_MAX_VERTEX_ATTRIBS );

	CVertexArray::VertexAttribPointer attrib;
	GLuint index;
	
	// Determine index
	index = m_vertexAttribsActive++;
	// Populate attrib struct
	attrib = { index, size, type, stride, pointer, normalized, internalType };

	return index;
}
GLuint CVertexArray::addVertexAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer, GLboolean normalized ) {
	return this->addVertexAttribInternal( CVertexArray::VertexAttribType::VAT_Special, size, type, stride, pointer, GL_FALSE );
}
GLuint CVertexArray::addVertexIAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer ) {
	return this->addVertexAttribInternal( CVertexArray::VertexAttribType::VAT_Integer, size, type, stride, pointer, GL_FALSE );
}
GLuint CVertexArray::addVertexLAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer ) {
	return this->addVertexAttribInternal( CVertexArray::VertexAttribType::VAT_Long, size, type, stride, pointer, GL_FALSE );
}

bool CVertexArray::flushBindsAndAttribs()
{
	assert( m_vaoId );
	assert( !m_buffersToBind.empty() );

	GLenum glError;

	this->bind();
	if( (glError = glGetError()) != GL_NO_ERROR ) {
		m_pGameHandle->getLogger()->printError( "Failed to bind vertex array, GL error code %u", glError );
		return false;
	}
	// Bind buffers waiting
	for( auto it: m_buffersToBind )
	{
		if( !it.second->bind( it.first ) ) {
			m_pGameHandle->getLogger()->printError( "Failed to bind buffer object, GL error code %u", glError );
			return false;
		}
	}
	// Create and enable vertex attribs waiting
	CVertexArray::VertexAttribPointer attrib;
	while( !m_vertexAttribQueue.empty() )
	{
		attrib = m_vertexAttribQueue.front();
		m_vertexAttribQueue.pop();

		switch( attrib.internalType )
		{
		case CVertexArray::VertexAttribType::VAT_Special:
			glVertexAttribPointer( attrib.index, attrib.size, attrib.type, attrib.normalized, attrib.stride, attrib.pointer );
			break;
		case CVertexArray::VertexAttribType::VAT_Integer:
			glVertexAttribIPointer( attrib.index, attrib.size, attrib.type, attrib.stride, attrib.pointer );
			break;
		case CVertexArray::VertexAttribType::VAT_Long:
			glVertexAttribLPointer( attrib.index, attrib.size, attrib.type, attrib.stride, attrib.pointer );
			break;
		default:
			assert( false );
			return false;
		}
		if( (glError = glGetError()) != GL_NO_ERROR ) {
			m_pGameHandle->getLogger()->printError( "Failed to call glVertexAttribPointer, GL error code %u", glError );
			return false;
		}
		// Enable vertex attrib
		glEnableVertexArrayAttrib( m_vaoId, attrib.index );
		if( (glError = glGetError()) != GL_NO_ERROR ) {
			m_pGameHandle->getLogger()->printError( "Failed to call glEnableVertexArrayAttrib, GL error code %u", glError );
			return false;
		}
	}

	return true;
}
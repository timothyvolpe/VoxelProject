#include <algorithm>
#include <glm/ext.hpp>
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

#ifdef _DEBUG
void CGraphics::MasterDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
{
	CGraphics *pGraphics = const_cast<CGraphics*>(reinterpret_cast<const CGraphics*>(userParam));
	pGraphics->debugCallback( source, type, id, severity, length, message );
}
#endif

CGraphics::CGraphics( CGame *pGameHandle )
{
	m_pGameHandle = pGameHandle;
	m_glSupportLevel = GLSupportLevel::GL_SUPPORT_NONE;
	m_pSDLWindow = 0;
	m_sdlContext = 0;

	m_pShaderManager = 0;

	m_projectionPerspMat = glm::mat4( 1.0f );
	m_projectionOrthoMat = glm::mat4( 1.0f );
	m_viewMat = std::make_shared<glm::mat4>( 1.0f );

	m_viewportOutOfDate = true;
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
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( CONFIG_STR_RESOLUTION_X, &resX );
	m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( CONFIG_STR_RESOLUTION_Y, &resY );

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

	// Iteratively try to create each GL context
	SDL_GLContext testContext = 0;
	for( char i = 1; i < GLSupportLevel::GL_SUPPORT_COUNT; i++ )
	{
		int attemptedMajor, attemptMinor;
		attemptedMajor = GLSupportVersion[i][0];
		attemptMinor = GLSupportVersion[i][1];

		// Delete previous
		if( testContext ) {
			SDL_GL_DeleteContext( testContext );
			testContext = 0;
		}

		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, attemptedMajor );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, attemptMinor );

		m_pGameHandle->getLogger()->print( "Attempting to create OpenGL %d.%d context...", attemptedMajor, attemptMinor );

		testContext = SDL_GL_CreateContext( m_pSDLWindow );
		if( !testContext ) {
			m_pGameHandle->getLogger()->printError( "Failed to create OpenGL context: %s", SDL_GetError() );
			break;
		}
		int versionMajor, versionMinor;
		SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &versionMajor );
		SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &versionMinor );
		m_pGameHandle->getLogger()->print( "Sucessfully created OpenGL %d.%d context!", versionMajor, versionMinor );

		m_sdlContext = testContext;
	}
	SDL_GL_MakeCurrent( m_pSDLWindow, m_sdlContext );

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

	// Set up debug output if supported
#ifdef _DEBUG
	if( glewIsSupported( "GL_ARB_debug_output" ) ) {
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		glEnable( GL_DEBUG_OUTPUT );
		glDebugMessageCallback( CGraphics::MasterDebugCallback, this );
		GLuint unusedIds = 0;
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE );
	}
#endif

	// Print GL info
	m_pGameHandle->getLogger()->print( "OpenGL Context Info:" );
	m_pGameHandle->getLogger()->print( "\tVersion: %s", glGetString( GL_VERSION ) );
	m_pGameHandle->getLogger()->print( "\tVendor: %s", glGetString( GL_VENDOR ) );
	m_pGameHandle->getLogger()->print( "\tRenderer: %s", glGetString( GL_RENDERER ) );
	m_pGameHandle->getLogger()->print( "\tGLSL: %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// GL Setup
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );
	glFrontFace( GL_CCW );

	glPointSize( 2.0f );

	glClearColor( 0, 0, 0.25f, 1.0f );

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
	// Update viewport if needed
	if( m_viewportOutOfDate ) {
		if( !this->setupViewport() )
			return false;
	}

	// Update matrix block
	glm::mat4 matrixBlock[3];
	UniformBlockData uboData = m_pShaderManager->getUniformBlock( UniformBlockIDs::UNIFORM_BLOCK_MATRIX );

	matrixBlock[0] = m_projectionPerspMat;
	matrixBlock[1] = m_projectionOrthoMat;
	matrixBlock[2] = (*m_viewMat);
	glBindBuffer( GL_UNIFORM_BUFFER, uboData.uboId );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( matrixBlock ), &matrixBlock[0] );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );

	// Clear screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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

	// Swap buffers
	SDL_GL_SwapWindow( m_pSDLWindow );

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

bool CGraphics::setupViewport()
{
	assert( m_pSDLWindow );

	unsigned int resolutionX, resolutionY;
	unsigned int refreshRate;
	float fov;
	WindowModes windowMode;
	SDL_DisplayMode displayMode;

	m_viewportOutOfDate = false;

	// Check the window mode
	if( !m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<int>( CONFIG_STR_WINDOW_MODE, reinterpret_cast<int*>(&windowMode) ) ) {
		windowMode = DEFAULT_WINDOW_MODE;
		m_pGameHandle->getLogger()->printWarn( "Failed to get window mode from config, using default" );
	}
	if( windowMode != WindowModes::WindowModeBordered
		&& windowMode != WindowModes::WindowModeBorderless
		&& windowMode != WindowModes::WindowModeFullscreen )
		windowMode = WindowModes::WindowModeBordered;
	// Handle switching between window modes
	switch( windowMode )
	{
	case WindowModes::WindowModeBordered:
		if( SDL_SetWindowFullscreen( m_pSDLWindow, 0 ) < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to exit fullscreen: %s", SDL_GetError() );
			return false;
		}
		SDL_SetWindowBordered( m_pSDLWindow, SDL_TRUE );
		break;
	case WindowModes::WindowModeBorderless:
		if( SDL_SetWindowFullscreen( m_pSDLWindow, 0 ) < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to exit fullscreen: %s", SDL_GetError() );
			return false;
		}
		SDL_SetWindowBordered( m_pSDLWindow, SDL_FALSE );
		break;
	case WindowModes::WindowModeFullscreen:
		if( SDL_SetWindowFullscreen( m_pSDLWindow, SDL_WINDOW_FULLSCREEN ) < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to enter fullscreen: %s", SDL_GetError() );
			return false;
		}
		break;
	default:
		assert( false );
		break;
	}

	// Get user config resolutions
	if( !m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( CONFIG_STR_RESOLUTION_X, &resolutionX ) ) {
		resolutionX = DEFAULT_RESOLUTION_X;
		m_pGameHandle->getLogger()->printWarn( "Failed to get resolution from config, using default" );
	}
	if( !m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( CONFIG_STR_RESOLUTION_Y, &resolutionY ) ) {
		resolutionY = DEFAULT_RESOLUTION_Y;
		m_pGameHandle->getLogger()->printWarn( "Failed to get resolution from config, using default" );
	}
	if( !m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<unsigned int>( CONFIG_STR_REFRESH_RATE, &refreshRate ) ) {
		resolutionY = DEFAULT_REFRESH_RATE;
		m_pGameHandle->getLogger()->printWarn( "Failed to get refresh rate from config, using default" );
	}

	// If fullscreen mode, try to find a video mode matching the user config
	if( WindowModes::WindowModeFullscreen )
	{
		// Get which window the display is in
		int displayIndex = SDL_GetWindowDisplayIndex( m_pSDLWindow );
		if( displayIndex < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to get display index: %s", SDL_GetError() );
			return false;
		}
		// Query display modes for current display
		int numDisplayModes = SDL_GetNumDisplayModes( displayIndex );
		if( numDisplayModes < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to get display modes: %s", SDL_GetError() );
			return false;
		}
		else if( numDisplayModes == 0 ) {
			m_pGameHandle->getLogger()->printError( "Did not find any fullscreen display modes!" );
			return false;
		}
		// Find a display mode that matches user config settings or use the last one we found
		for( int i = 0; i < numDisplayModes; i++ )
		{
			if( SDL_GetDisplayMode( displayIndex, i, &displayMode ) < 0 )
				m_pGameHandle->getLogger()->printWarn( "Failed to query a display mode: %s", SDL_GetError() );
			else {
				// Check if we found a match
				if( displayMode.w == resolutionX && displayMode.h == resolutionY && displayMode.refresh_rate == 60 )
					break;
			}
			if( i == numDisplayModes-1 )
				m_pGameHandle->getLogger()->printWarn( "Could not find a display mode matching %dx%d px @ %d Hz", resolutionX, resolutionY, refreshRate );
		}
		// Update display mode
		resolutionX = displayMode.w;
		resolutionY = displayMode.h;
		refreshRate = displayMode.refresh_rate;
		m_pGameHandle->getLogger()->print( "Setting fullscreen to %dx%d px @ %d Hz", resolutionX, resolutionY, refreshRate );
		if( SDL_SetWindowDisplayMode( m_pSDLWindow, &displayMode ) < 0 ) {
			m_pGameHandle->getLogger()->printError( "Failed to set display mode: %s", SDL_GetError() );
			return false;
		}
		// Update config to reflect valid display mode
		m_pGameHandle->getClient()->getClientConfig()->updateProperty<unsigned int>( CONFIG_STR_RESOLUTION_X, resolutionX );
		m_pGameHandle->getClient()->getClientConfig()->updateProperty<unsigned int>( CONFIG_STR_RESOLUTION_Y, resolutionY );
		m_pGameHandle->getClient()->getClientConfig()->updateProperty<unsigned int>( CONFIG_STR_REFRESH_RATE, refreshRate );
	}
	// If window mode, set resolution
	else
		SDL_SetWindowSize( m_pSDLWindow, resolutionX, resolutionY );

	if( !m_pGameHandle->getClient()->getClientConfig()->getPropertyFromConfig<float>( CONFIG_STR_FOV, &fov ) ) {
		fov = DEFAULT_FOV;
		m_pGameHandle->getLogger()->printWarn( "Failed to get FOV from config, using default" );
	}
	fov = glm::clamp( fov, FOV_MIN, FOV_MAX );

	m_projectionPerspMat = glm::perspective( fov, (float)(resolutionX / resolutionY), 0.1f, 100.0f );

	//glViewport( 0, 0, resolutionX, resolutionY );

	return true;
}

#ifdef _DEBUG
void CGraphics::debugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message )
{
	// Determine severity
	std::string severityStr;
	switch( severity )
	{
	case GL_DEBUG_SEVERITY_LOW:
		severityStr = "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severityStr = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		severityStr = "HIGH";
		break;
	default:
		severityStr = "UNKNOWN";
		break;
	}
	// Determine message type
	switch( type )
	{
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		m_pGameHandle->getLogger()->printError( "OPENGL UNDEFINED (id: %u, s: %s): %s", id, severityStr.c_str(), message );
		break;
	case GL_DEBUG_TYPE_ERROR:
		m_pGameHandle->getLogger()->printError( "OPENGL ERROR (id: %u, s: %s): %s", id, severityStr.c_str(), message );
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
	case GL_DEBUG_TYPE_PERFORMANCE:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		m_pGameHandle->getLogger()->printWarn( "OPENGL WARNING (id: %u, s: %s): %s", id, severityStr.c_str(), message );
		break;
	case GL_DEBUG_TYPE_OTHER:
		m_pGameHandle->getLogger()->print( "OPENGL (id: %u, s: %s): %s", id, severityStr.c_str(), message );
		break;
	}
	
}
#endif

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
	m_bufferData = 0;
	m_bufferFlags = 0;
	m_bufferUsage = 0;
}
CBufferObject::~CBufferObject()
{
	this->destroy();
}

void CBufferObject::create( GLsizeiptr size, void* data, GLbitfield flags, GLenum usage )
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
			glBufferStorage( target, m_bufferSize, m_bufferData, m_bufferFlags );
		else 
			glBufferData( target, m_bufferSize, m_bufferData, m_bufferUsage );
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

	m_vertexAttribQueue.push( attrib );

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
		glEnableVertexAttribArray( attrib.index );
		if( (glError = glGetError()) != GL_NO_ERROR ) {
			m_pGameHandle->getLogger()->printError( "Failed to call glEnableVertexArrayAttrib, GL error code %u", glError );
			return false;
		}
	}

	return true;
}
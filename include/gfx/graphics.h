/**
* @file graphics.h
* @brief Contains the CGraphics class, which handles the OpenGL context.
* @details The class manages all of the rendering functions and rendering subclasses.
*
* @author Timothy Volpe
* @date 12/15/2019
*/

#pragma once

#define OPENGL_RED_BITS 5
#define OPENGL_GREEN_BITS 5
#define OPENGL_BLUE_BITS 5
#define OPENGL_DEPTH_BITS 16

#define DEFAULT_RESOLUTION_X 1280
#define DEFAULT_RESOLUTION_Y 720
#define DEFAULT_REFRESH_RATE 60
#define DEFAULT_FOV 60.0f

#define FOV_MIN 30.0f
#define FOV_MAX 120.0f

#include <SDL.h>
#include <gl\glew.h>
#include <GL\GL.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <queue>
#include <assert.h>

class CGame;
class CShaderManager;
class CVertexArray;

/**
* @brief Defines which feature sets are support on the client computer
*/
enum GLSupportLevel : char
{
	/** No support, the game cannot run the client on this computer/ */
	GL_SUPPORT_NONE = 0,
	/** Minimum support, the computer supports OpenGL 3.3+ */
	GL_SUPPORT_MIN = 1,
	/** Standard support, the computer supports OpenGL 4.1+ */
	GL_SUPPORT_STD = 2,
	/** Maximum support, the computer supports OpenGL 4.6+ */
	GL_SUPPORT_MAX = 3,
	/** Number of support levels */
	GL_SUPPORT_COUNT
};

/** Defines which version each support level represents, index corresponds to GLSupportLevel value. */
static const int GLSupportVersion[][2] ={ {0,0}, {3,3}, {4,1}, {4,6} };
/** The GLSL version defines which shaders to load */
static const int GLSLVersion[] ={ 0, 330, 410, 460 };

struct RenderJob
{
	std::shared_ptr<CVertexArray> vertexArray;
	unsigned int shaderIndex;
	unsigned int vertexCount;
};
typedef std::pair<GLuint, unsigned int> ArrayShaderPair;
bool RenderJobSort( RenderJob& a, RenderJob& b );

enum WindowModes
{
	WindowModeBordered = 0,
	WindowModeBorderless = 1,
	WindowModeFullscreen = 2
};

#define DEFAULT_WINDOW_MODE WindowModes::WindowModeBordered

/**
* @brief The graphics handler.
* @details This class is in charge of managing the OpenGL context and all its functionality, as well as
*	handling all the rendering subclasses.
*
* @author Timothy Volpe
* @date 12/15/2019
*/
class CGraphics
{
private:
	static int SDLReferenceCount;
	static bool GLEWInitialized;

	CGame *m_pGameHandle;

	char m_glSupportLevel;

	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_sdlContext;

	CShaderManager *m_pShaderManager;

	std::vector<RenderJob> m_renderJobs;

	glm::mat4 m_projectionPerspMat, m_projectionOrthoMat;
	std::shared_ptr<glm::mat4> m_viewMat;

	bool m_viewportOutOfDate;

	/**
	* @brief This sets up, or reconfigs, the viewport. This means the window and the rendering context.
	* @details If we are in full screen, the user can set the resolution to any of the valid resolutions, and the aspect ratio will suffer if it does not match the monitor.
	*	If we are in windowed mode, the user can set the resolution to any value and the window will change size to accomadate.
	* @returns True if successfully set video mode, false if there was an issue with the video modes.
	*/
	bool setupViewport();
public:
#ifdef _DEBUG
	/**
	* @brief See debugCallback
	* @details Reroutes to the debugCallback of the CGraphics class pointer stored in userParam
	*/
	static void MasterDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam );
#endif

	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	CGraphics( CGame *pGameHandle );
	/**
	* @brief Destructor.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	~CGraphics();

	/**
	* @brief Initializes the graphics class.
	* @return True if successfully initialized, false if otherwise. Error messagebox will be displayed,
	*		with associated log output.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	bool initialize();
	/**
	* @brief Destroys client data, and deletes pointers
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	void destroy();

	/**
	* @brief Draw the next frame.
	* @details Performs the necessary OpenGL/SDL steps to draw a frame, as well as notify all the 
	* rendering classes that it is time to draw.
	*/
	bool draw();

	
#ifdef _DEBUG
	/**
	* @brief The debug callback for openGL 4.3+ in debug mode
	* @details See glDebugMessageCallback
	*/
	void debugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message );
#endif

	/**
	* @brief Submits a vertex array for rendering when the frame is drawn.
	* @details The information passed to this function is saved and used to draw the frame. It is cleared after the draw call.
	* @param[in]	vertexArray		Pointer to the vertex array to render.
	* @param[in]	shaderIndex		Index of the shader to render with, retrieved from the shader manager.
	* @param[in]	vertexCount		The number of vertices to render
	*/
	void submitForDraw( std::shared_ptr<CVertexArray> vertexArray, unsigned int shaderIndex, unsigned int vertexCount );

	/**
	* @brief Get shader manager.
	* @returns The shader manager.
	*/
	inline CShaderManager* getShaderManager() { return m_pShaderManager; }

	inline char getGLSupportLevel() { return m_glSupportLevel; }
	inline int getGLSLVersion() { assert( m_glSupportLevel >= 0 && m_glSupportLevel < GLSupportLevel::GL_SUPPORT_COUNT ); return GLSLVersion[m_glSupportLevel]; }

	/**
	* @brief Get a shared pointer to the view matrix.
	* @details Only sent to the shaders once at the beginning of the frame.
	* @returns A shared pointer to the view matrix.
	*/
	inline std::shared_ptr<glm::mat4> getViewMatrixPtr() { return m_viewMat; }
};

enum OpenGLBufferTypes : uint16_t
{
	ArrayBuffer					= 1,
	AtomicCounterBuffer			= 1 << 2,
	CopyReadBuffer				= 1 << 3,
	CopyWriteBuffer				= 1 << 4,
	DispatchIndirectBuffer		= 1 << 5,
	DrawIndirectBuffer			= 1 << 6,
	ElementArrayBuffer			= 1 << 7,
	PixelPackBuffer				= 1 << 8,
	PixelUnpackBuffer			= 1 << 9,
	QueryBuffer					= 1 << 10,
	ShaderStorageBuffer			= 1 << 11,
	TextureBuffer				= 1 << 12,
	TransformFeedbackBuffer		= 1 << 13,
	UniformBuffer				= 1 << 14
};

/**
* @brief A wrapper for an OpenGL buffer object
*
* @author Timothy Volpe
* @date 4/29/2020
*/
class CBufferObject
{
private:
	GLuint m_bufferId;

	GLenum m_bufferTarget;
	GLsizeiptr m_bufferSize;
	void* m_bufferData;
	GLbitfield m_bufferFlags;
	GLenum m_bufferUsage;
public:
	CBufferObject();
	~CBufferObject();

	/**
	* @brief Creates the buffer data.
	* @details This does not actually create the openGL buffer object. The object is not created until the buffer is bound to a vertex array object.
	*	This function saves the arguments passed to it and uses them to create the buffer object when it is bound to its first VAO.
	*	See glBufferData and glBufferStorage for information on the parameters.
	* @warning This means data will not be used until the buffer is bound.
	*/
	void create( GLsizeiptr size, void* data, GLbitfield flags, GLenum usage );
	/**
	* @brief Destroys the opengl buffer and frees its resources
	*/
	void destroy();

	/**
	* @brief Bind the vertex buffer object to a specific target
	* @details If the buffer has not been created, it will be created now. It cannot be resized once it has been created,
	*	it must be destroyed and recreated in order to be resized.
	* @param[in]	target	The target to bind to, see OpenGL documentation for glBindBuffer
	* @returns True if successfully bound buffer (and created if necessary), false otherwise.
	*/
	bool bind( GLenum target );

	inline const GLuint getBufferId() { return m_bufferId; }
};

/**
* @brief A wrapper for an OpenGL Vertex Array Object (VAO)
*
* @author Timothy Volpe
* @date 4/27/2020
*/
class CVertexArray
{
private:
	enum VertexAttribType
	{
		VAT_Special,	// glVertexAttribPointer
		VAT_Integer,	// glVertexAttribIPointer
		VAT_Long		// glVertexAttribLPointer
	};
	struct VertexAttribPointer
	{
		GLuint index;
		GLint size;
		GLenum type;
		GLsizei stride;
		const void* pointer;

		GLboolean normalized;

		unsigned char internalType;
	};

	typedef std::pair<GLenum, std::shared_ptr<CBufferObject>> BufferPair;

	CGame* m_pGameHandle;

	GLuint m_vaoId;

	std::vector<BufferPair> m_buffersToBind;
	std::vector<BufferPair> m_boundBuffers;
	std::queue<CVertexArray::VertexAttribPointer> m_vertexAttribQueue;

	unsigned int m_vertexAttribsActive;

	GLuint addVertexAttribInternal( unsigned char internalType, GLint size, GLenum type, GLsizei stride, const void *pointer, GLboolean normalized );
public:
	CVertexArray( CGame* pGameHandle );
	~CVertexArray();

	/**
	* @brief Creates the opengl vertex array object.
	* @details VAO must not have been previously created.
	* @returns True if successfully created the VAO, false if there was a failure.
	*/
	bool create();
	/**
	* @brief Destroys the VAO and frees its resources.
	*/
	void destroy();

	/**
	* @brief Bind the VAO object.
	*/
	void bind();

	/**
	* @brief Add a buffer object to the bind queue. Must flush to bind.
	* @details This will not automatically bind the buffer. To reduce calls to glBindVertexArray, the buffer will be bound when
	*	flushBindsAndAttribs is called.
	* @param[in]	bufferObject	The buffer object to bind.
	* @param[in]	target			The target to bind the buffer object to.
	*/
	void addBuffer( std::shared_ptr<CBufferObject> bufferObject, GLenum target );

	/**
	* @brief Adds a vertex attrib to the vertex array object.
	* @details This will not automatically add the vertex attribute. To reduce calls to glBindVertexArray, the attributes will be bound when 
	*	flushBindsAndAttribs is called. See glVertexAttribPointer for parameter descriptions.
	* @returns The index of the added vertex attribute.
	*/
	GLuint addVertexAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer, GLboolean normalized );

	/**
	* @brief See addVertexAttrib, performs the same function but for glVertexAttribIPointer.
	*/
	GLuint addVertexIAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer );
	/**
	* @brief See addVertexAttrib, performs the same function but for glVertexAttribLPointer.
	*/
	GLuint addVertexLAttrib( GLint size, GLenum type, GLsizei stride, const void *pointer );

	/**
	* @brief Binds all the buffer objects waiting to be bound, and adds vertex attribs.
	* @returns True if successfully bound all buffers and vertex attribs, false if something failed.
	*/
	bool flushBindsAndAttribs();

	/**
	* @brief Get the vertex array id.
	* @returns Vertex array object id.
	*/
	const GLuint getVertexArrayId() { return m_vaoId; }
};
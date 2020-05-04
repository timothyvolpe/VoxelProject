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

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1

#include <SDL.h>
#include <gl\glew.h>
#include <GL\GL.h>
#include <memory>
#include <vector>

class CGame;
class CShaderManager;

/**
* @brief Defines which feature sets are support on the client computer
*/
enum GLSupportLevel : char
{
	/** No support, the game cannot run the client on this computer/ */
	GL_SUPPORT_NONE = 0,
	/** Minimum support, the computer supports OpenGL 3.2+ */
	GL_SUPPORT_MIN = 1,
	/** Standard support, the computer supports OpenGL 4.1+ */
	GL_SUPPORT_STD = 2,
	/** Maximum support, the computer supports OpenGL 4.6+ */
	GL_SUPPORT_MAX = 3
};

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
public:
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
public:
	CBufferObject();
	~CBufferObject();

	/**
	* @brief Creates the opengl buffer.
	* @details Buffer must not have been previously created.
	* @returns True if successfully created the buffer, false if there was a failure.
	*/
	bool create();
	/**
	* @brief Destroys the opengl buffer and frees its resources
	*/
	void destroy();

	/**
	* @brief Bind the vertex buffer object to a specific target
	* @param[in]	target	The target to bind to, see OpenGL documentation for glBindBuffer
	*/
	void bind( GLenum target );

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
	typedef std::pair<GLenum, std::shared_ptr<CBufferObject>> BufferPair;

	CGame* m_pGameHandle;

	GLuint m_vaoId;

	std::vector<BufferPair> m_buffersToBind;
	std::vector<BufferPair> m_boundBuffers;
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
	* @details Must call flushBinds to actually bind the buffer to the vao
	* @param[in]	bufferObject	The buffer object to bind.
	* @param[in]	target			The target to bind the buffer object to.
	*/
	void addBuffer( std::shared_ptr<CBufferObject> bufferObject, GLenum target );

	/**
	* @brief Binds all the buffer objects added with addBuffer and clears queue.
	* @returns True if successfully bound all buffers, false if something failed.
	*/
	bool flushBinds();
};
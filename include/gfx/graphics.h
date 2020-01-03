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

class CGame;

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

	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_sdlContext;
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

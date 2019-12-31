/**
* @file graphics.h
* @brief Contains the CGraphics class, which handles the OpenGL context.
* @details The class manages all of the rendering functions and rendering subclasses.
*
* @author Timothy Volpe
* @date 12/15/2019
*/

#pragma once

#include <SDL.h>

class CGame;
class CWindow;

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
	CGame *m_pGameHandle;
	
	CWindow *m_pWindow;
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
};

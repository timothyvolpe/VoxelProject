/**
* @file window.h
* @brief Contains the CWindow class, which handles the rendering window
* @details Manages the SDL window object.
*
* @author Timothy Volpe
* @date 12/15/2019
*/

#pragma once

#include <SDL.h>

/**
* @brief The rendering window handler.
* @details In charge of managing the SDL window object. 
*
* @author Timothy Volpe
* @date 12/15/2019
*/
class CWindow
{
private:
	SDL_Window *m_pSDLWindow;
public:
	/**
	* @brief Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	CWindow();
	/**
	* @brief Destructor.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	~CWindow();

	/**
	* @brief Initializes the window class.
	* @details Creates the SDL window object. The window is centered by default, and the window size is that which was last saved
	*	in the config files.
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
	* @brief Returns the SDL window object.
	* @return Returns the SDL window object.
	* @author Timothy Volpe
	* @date 12/15/2019
	*/
	SDL_Window* getSDLWindow();
};

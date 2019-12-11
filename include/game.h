#pragma once

/// The main game class, everything starts and ends here.
/**
* It should be created in the main function.
* It is not recommended to have more than one game class, as there will be conflicts accessing the 
* game files.
*
* @author Timothy Volpe
* @date 12/10/2019
*/
class CGame
{
public:

	/**
	* Constructor. Initializes all variables to NULL or 0.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	CGame();
	/**
	* Destructor. Deletes all pointers.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	~CGame();

	/**
	* Initializes the game class. Creates and initializes game objects.
	* @return True is successfully initialized, False if otherwise. Error messagebox will be displayed,
	*		with associated log output.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	bool initialize();
	/**
	* Destroys game objects, but does not delete pointers.
	* @author Timothy Volpe
	* @date 12/10/2019
	*/
	void destroy();
};
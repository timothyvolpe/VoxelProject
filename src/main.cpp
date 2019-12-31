#include <iostream>
#include "game.h"
#include "logger.h"

int main( int argc, char *argv[] )
{
	CGame game;

	if( !game.initialize() )
		return 1;

	// Start the game!
	game.startGame();

	game.destroy();

	return 0;
}
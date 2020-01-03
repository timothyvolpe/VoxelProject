#include <iostream>
#include "game.h"
#include "logger.h"

int main( int argc, char *argv[] )
{
	CGame game;
	bool success;

	success = game.initialize();

	// Start the game!
	if( success )
		game.startGame();

	game.destroy();

	if( !success )
		return 1;
	return 0;
}
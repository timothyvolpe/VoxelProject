#include <iostream>
#include "game.h"
#include "logger.h"

int main( int argc, char *argv[] )
{
	CGame game;

	if( !game.initialize() )
		return 1;

	game.getLogger()->print( "Hello World %d %d %s", 1, 2, "yes" );
	game.getLogger()->printWarn( "Hello World %d %d %s", 1, 2, "yes" );
	game.getLogger()->printError( "Hello World %d %d %s", 1, 2, "yes" );
	game.getLogger()->printLua( "Hello World %d %d %s", 1, 2, "yes" );
	game.getLogger()->printLuaError( "Hello World %d %d %s", 1, 2, "yes" );
	game.getLogger()->fatalMessageBox( "Fuck" );

	game.destroy();

	return 0;
}
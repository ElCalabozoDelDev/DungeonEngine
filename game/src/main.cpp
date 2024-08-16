#include "core/game.hpp"

int WinMain( int argc, char* args[] )
{
    Game::Instance()->init("EnTT + SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, false);
	Game::Instance()->run();
    return 0;
}

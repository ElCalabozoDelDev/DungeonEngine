#include "core/game.hpp"

const int width = 800;
const int height = 600;
const int FPS = 60;
const int frameDelay = 1000 / FPS;
int WinMain( int argc, char* args[] )
{
    Game::Instance()->init("DungeonEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, false, FPS, frameDelay);
	Game::Instance()->run();
    return 0;
}

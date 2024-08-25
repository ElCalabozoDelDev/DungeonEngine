#define SDL_MAIN_HANDLED
#include "core/game.hpp"


int main( int argc, char* args[] )
{
    Game::Instance()->init("../assets/game.xml");
    Game::Instance()->run();
    return 0;
}

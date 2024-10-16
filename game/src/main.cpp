#define SDL_MAIN_HANDLED
// #include "core/game.hpp"

// int main( int argc, char* args[] )
// {
//     Game::Instance()->init("../assets/game.xml");
//     Game::Instance()->run();
//     return 0;
// }

#include "core/game_loop.hpp"
#include "loaders/config.hpp"
#include "plugins/base_plugin.hpp"
#include "plugins/game_plugin.hpp"


int main(int argc, char* argv[]) {
	GameLoop gameLoop;
	BasePlugin basePlugin("../assets/game.xml");
	GamePlugin gamePlugin;
	gameLoop.addPlugin(basePlugin);
	gameLoop.addPlugin(gamePlugin);
	gameLoop.run();
	return 0;
}
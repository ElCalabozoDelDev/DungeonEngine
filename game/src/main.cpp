#define SDL_MAIN_HANDLED
// #include "core/game.hpp"


// int main( int argc, char* args[] )
// {
//     Game::Instance()->init("../assets/game.xml");
//     Game::Instance()->run();
//     return 0;
// }

#include "core/game_loop.hpp"
#include "plugins/base_plugin.hpp"
#include "plugins/game_plugin.hpp"
#include "core/config.hpp"
#include "core/config_loader.hpp"
int main(int argc, char *argv[])
{
    GameLoop gameLoop;
    Config config;
    ConfigLoader::loadConfigFromXML("../assets/game.xml", config);
    BasePlugin basePlugin(config);
    GamePlugin gamePlugin;
    gameLoop.addPlugin(basePlugin);
    gameLoop.addPlugin(gamePlugin);
    gameLoop.run();
    return 0;
}
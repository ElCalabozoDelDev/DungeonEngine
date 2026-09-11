#define SDL_MAIN_HANDLED

#include <engine/core/game_loop.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <game/plugins/game_plugin.hpp>

int main(int /*argc*/, char* /*argv*/[])
{
    de::GameLoop gameLoop;
    de::BasePlugin basePlugin("../assets/game.xml");
    GamePlugin gamePlugin;

    gameLoop.addPlugin(basePlugin);
    gameLoop.addPlugin(gamePlugin);
    gameLoop.run();

    return 0;
}

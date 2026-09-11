#define SDL_MAIN_HANDLED

#include <engine/core/asset_paths.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config_loader.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <game/plugins/game_plugin.hpp>
#include <iostream>
#include <memory>

int main(int /*argc*/, char* /*argv*/[])
{
    // Assets are found relative to the executable, so the game runs from any
    // working directory -- including a double-click from the file explorer.
    auto assets = de::AssetPaths::discover();
    if (!assets)
    {
        std::cerr << "Assets: " << assets.error() << std::endl;
        return 1;
    }

    auto config = de::ConfigLoader::load(assets->resolve("game.xml"));
    if (!config)
    {
        std::cerr << "Config: " << config.error() << std::endl;
        return 1;
    }

    de::GameLoop gameLoop;
    gameLoop.addPlugin(
        std::make_unique<de::BasePlugin>(std::move(*config), *assets));
    gameLoop.addPlugin(std::make_unique<GamePlugin>());

    return gameLoop.run() ? 0 : 1;
}

#define SDL_MAIN_HANDLED

#include <cstdlib>
#include <cstring>
#include <engine/core/asset_paths.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config_loader.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/scene/in_game_scene.hpp>
#include <iostream>
#include <memory>
#include <string_view>

namespace
{
struct Options
{
    /// Exit after this many frames. 0 means run until the player quits.
    /// Exists so CI can run the whole startup path headlessly.
    int frames = 0;
    /// Start in the level instead of the menu.
    bool skipMenu = false;
};

Options parseArguments(int argc, char* argv[])
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument = argv[i];
        if (argument == "--frames" && i + 1 < argc)
        {
            options.frames = std::atoi(argv[++i]);
        }
        else if (argument == "--level")
        {
            options.skipMenu = true;
        }
        else
        {
            std::cerr << "Unknown option: " << argument << "\n"
                      << "Usage: DungeonEngine [--frames N] [--level]\n";
        }
    }
    return options;
}

} // namespace

int main(int argc, char* argv[])
{
    const Options options = parseArguments(argc, argv);

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

    if (options.skipMenu)
    {
        gameLoop.addSetupCallback(
            [](entt::registry& registry)
            {
                if (auto* scenes = registry.ctx().find<de::SceneSystem>())
                {
                    scenes->requestScene(std::make_unique<InGameScene>());
                }
            });
    }

    if (options.frames > 0)
    {
        gameLoop.addFrameEndCallback(
            [remaining = options.frames](entt::registry& registry) mutable
            {
                if (--remaining <= 0)
                {
                    registry.ctx().get<de::ControlFlow>() =
                        de::ControlFlow::Exit;
                }
            });
    }

    return gameLoop.run() ? 0 : 1;
}

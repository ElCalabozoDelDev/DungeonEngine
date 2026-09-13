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
#include <game/sim/sim_options.hpp>
#include <game/sim/sim_plugin.hpp>
#include <iostream>
#include <memory>
#include <string_view>

namespace
{
struct Options
{
    int frames = 0;
    bool skipMenu = false;
    bool sim = false;
    SimOptions simOptions;
};

constexpr std::string_view Usage =
    "Usage: DungeonEngine [--frames N] [--level]\n"
    "                     [--sim [--sim-out PATH] [--sim-summary PATH]\n"
    "                            [--sim-steps N] [--sim-seed N]\n"
    "                            [--sim-require-clear] [--sim-window]]\n";

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
        else if (argument == "--sim")
        {
            options.sim = true;
        }
        else if (argument == "--sim-out" && i + 1 < argc)
        {
            options.simOptions.csvPath = argv[++i];
        }
        else if (argument == "--sim-summary" && i + 1 < argc)
        {
            options.simOptions.summaryPath = argv[++i];
        }
        else if (argument == "--sim-steps" && i + 1 < argc)
        {
            options.simOptions.maxSteps = std::atoi(argv[++i]);
        }
        else if (argument == "--sim-seed" && i + 1 < argc)
        {
            options.simOptions.seed =
                static_cast<unsigned int>(std::atoi(argv[++i]));
        }
        else if (argument == "--sim-require-clear")
        {
            options.simOptions.requireClear = true;
        }
        else if (argument == "--sim-window")
        {
            options.simOptions.window = true;
        }
        else
        {
            std::cerr << "Unknown option: " << argument << "\n" << Usage;
        }
    }

    if (options.sim)
    {
        options.skipMenu = true;
    }
    return options;
}

} // namespace

int main(int argc, char* argv[])
{
    const Options options = parseArguments(argc, argv);

    auto assets = de::AssetPaths::discover();
    if (!assets)
    {
        std::cerr << "Assets: " << assets.error() << std::endl;
        return 1;
    }

    auto config = de::ConfigLoader::load(assets->resolve("game.json"));
    if (!config)
    {
        std::cerr << "Config: " << config.error() << std::endl;
        return 1;
    }

    de::GameLoop gameLoop;
    gameLoop.addPlugin(
        std::make_unique<de::BasePlugin>(std::move(*config), *assets));
    gameLoop.addPlugin(std::make_unique<GamePlugin>());

    SimPlugin* sim = nullptr;
    if (options.sim)
    {
        auto plugin = std::make_unique<SimPlugin>(options.simOptions);
        sim = plugin.get();
        gameLoop.addPlugin(std::move(plugin));
        gameLoop.setFrameDelta(de::DeltaTime{}.fixed);
    }

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

    if (!gameLoop.run() || (sim != nullptr && sim->failed()))
    {
        return 1;
    }

    if (sim != nullptr && options.simOptions.requireClear && !sim->cleared())
    {
        return 2;
    }
    return 0;
}

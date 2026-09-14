#define SDL_MAIN_HANDLED

#include <engine/core/asset_paths.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config_loader.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/command_line.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/sim/sim_plugin.hpp>
#include <iostream>
#include <memory>
#include <span>

int main(int argc, char* argv[])
{
    const auto parsed = parseCommandLine(std::span<const char* const>(
        argv + 1, static_cast<std::size_t>(argc - 1)));
    if (!parsed)
    {
        // 2 for a bad command line, so a script can tell it from a run that
        // started and failed (1).
        std::cerr << parsed.error() << "\n" << CommandLineUsage;
        return 2;
    }
    const CommandLine& options = *parsed;

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
    return 0;
}

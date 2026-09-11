#include <SDL_events.h>
#include <engine/scene/scene_system.hpp>
#include <engine/systems/camera_system.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/systems/movement_system.hpp>
#include <imgui/imgui_impl_sdl2.h>
#include <memory>

using namespace de;

void GamePlugin::mount(de::GameLoop& gameLoop)
{
    auto debugSystem = std::make_shared<DebugSystem>();
    auto sceneSystem = std::make_shared<SceneSystem>();

    gameLoop.addSetupCallback(
        [sceneSystem, debugSystem](entt::registry& registry)
        {
            registry.ctx().emplace<std::shared_ptr<SceneSystem>>(sceneSystem);
            registry.ctx().emplace<std::shared_ptr<DebugSystem>>(debugSystem);
            sceneSystem->changeScene(registry, std::make_unique<InGameScene>());
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto& cf = registry.ctx().get<ControlFlow>();
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    cf = ControlFlow::Exit;
                }
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
        });

    // Input-driven movement is gameplay, so it is mounted here rather than by
    // the engine's BasePlugin.
    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

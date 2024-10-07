#include "plugins/game_plugin.hpp"
#include "SDL_events.h"
#include "imgui/imgui_impl_sdl2.h"
#include "systems/camera_system.hpp"
#include "systems/scene_system.hpp"
#include "scene/in_game_scene.hpp"
#include "systems/debug_system.hpp"
#include <memory>

void GamePlugin::mount(GameLoop &gameLoop)
{
    auto debugSystem = std::make_shared<DebugSystem>();
    auto sceneSystem = std::make_shared<SceneSystem>();
    gameLoop.addSetupCallback([sceneSystem, debugSystem](entt::registry &registry)
                              {
            registry.ctx().emplace<std::shared_ptr<SceneSystem>>(sceneSystem);
            registry.ctx().emplace<std::shared_ptr<DebugSystem>>(debugSystem);
            sceneSystem->changeScene(registry, std::make_unique<InGameScene>()); });


    gameLoop.addFrameBeginCallback([sceneSystem](entt::registry &registry)
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
            } });

    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}
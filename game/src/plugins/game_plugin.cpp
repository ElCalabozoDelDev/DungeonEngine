#include "plugins/game_plugin.hpp"
#include "SDL_events.h"
#include "systems/widget_system.hpp"
#include "systems/scene_system.hpp"
#include "scene/in_game_scene.hpp"

void GamePlugin::mount(GameLoop &gameLoop)
{
    auto sceneSystem = std::make_shared<SceneSystem>();
    gameLoop.addSetupCallback([sceneSystem](entt::registry &registry)
                              {
            registry.ctx().emplace<std::shared_ptr<SceneSystem>>(sceneSystem);
            sceneSystem->changeScene(registry, std::make_unique<InGameScene>()); });

    gameLoop.addFrameBeginCallback([sceneSystem](entt::registry &registry)
                                   {
            auto& cf = registry.ctx().get<ControlFlow>();
            auto& imGuiSystem = *registry.ctx().get<std::shared_ptr<WidgetSystem>>();
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                imGuiSystem.handle(event);
                if (event.type == SDL_QUIT)
                {
                    cf = ControlFlow::Exit;
                }
            } });

    gameLoop.addSystem(sceneSystem);
}
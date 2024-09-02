#include "plugins/game_plugin.hpp"

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
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                // m_imguiSystem.handleEvents(event);
                if (event.type == SDL_QUIT)
                {
                    cf = ControlFlow::Exit;
                }
            } });

    gameLoop.addSystem(sceneSystem);
}
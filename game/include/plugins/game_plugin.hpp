#ifndef GAME_PLUGIN_HPP
#define GAME_PLUGIN_HPP

#include "plugin.hpp"
#include "systems/scene_system.hpp"
#include "scene/in_game_scene.hpp"
#include "core/game_loop.hpp"

class GamePlugin : public Plugin
{
public:
    void mount(GameLoop &gameLoop) override
    {
        auto sceneManager = std::make_shared<SceneSystem>();

        gameLoop.addSetupCallback([sceneManager](entt::registry &registry)
                                  {
            registry.ctx().emplace<std::shared_ptr<SceneSystem>>(sceneManager);
            sceneManager->changeScene(registry, std::make_unique<InGameScene>()); });

        gameLoop.addFrameBeginCallback([sceneManager](entt::registry &registry)
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
            } 
        });

        gameLoop.addSystem(sceneManager);
    }
};

#endif
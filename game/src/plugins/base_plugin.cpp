#include "plugins/base_plugin.hpp"

void BasePlugin::mount(GameLoop &gameLoop) {
    gameLoop.addPlugin(m_sdl);
    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }
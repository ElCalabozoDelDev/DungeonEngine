#include "plugins/base_plugin.hpp"
#include "systems/movement_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/update_animation_system.hpp"

void BasePlugin::mount(GameLoop &gameLoop) {
    gameLoop.addPlugin(m_sdl);
    gameLoop.addPlugin(m_imgui);
    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }
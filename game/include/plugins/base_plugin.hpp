#ifndef BASE_PLUGIN_HPP
#define BASE_PLUGIN_HPP

#include "plugin.hpp"
#include <SDL.h>
#include <SDL_stdinc.h>
#include <memory>
#include <string>
#include <entt/entt.hpp>
#include "core/game_loop.hpp"
#include "systems/movement_system.hpp" // Include the header file for EventSystem
#include "systems/render_system.hpp" // Include the header file for RenderSystem
#include "systems/transform_system.hpp"
#include "systems/update_animation_system.hpp"
#include "core/config.hpp"
#include "plugins/sdl_plugin.hpp"
class BasePlugin final : public Plugin {
public:

private:
  SDLPlugin m_sdl;
public:
  BasePlugin(const Config &config) : m_sdl(config) {}

  void mount(GameLoop &gameLoop) override {
    gameLoop.addPlugin(m_sdl);
    gameLoop.addSetupCallback([this](entt::registry &registry) {

    });

    gameLoop.addTeardownCallback([](entt::registry &registry) {

    });

    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }
};

#endif
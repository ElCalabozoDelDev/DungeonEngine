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
class BasePlugin final : public Plugin {
public:
  struct Config {
    std::string windowTitle;
    Uint32 frameRateCap;
  };

private:
  Config m_config;

public:
  BasePlugin(const Config &config) : m_config(config) {}

  void mount(GameLoop &gameLoop) override {
    gameLoop.addSetupCallback([this](entt::registry &registry) {
      SDL_Init(SDL_INIT_EVERYTHING);

      SDL_Window *window =
          SDL_CreateWindow(m_config.windowTitle.c_str(), SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
      SDL_Renderer *renderer =
          SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      
      registry.ctx().emplace<SDL_Window *>(window);
      registry.ctx().emplace<SDL_Renderer *>(renderer);
    });

    gameLoop.addTeardownCallback([](entt::registry &registry) {
      SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
      SDL_Window *window = registry.ctx().get<SDL_Window *>();

      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
    });

    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }
};

#endif
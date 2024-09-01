#ifndef BASE_PLUGIN_HPP
#define BASE_PLUGIN_HPP

#include "plugin.hpp"
#include <SDL.h>
#include <SDL_stdinc.h>

class BasePlugin : public Plugin {
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
          SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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

    // gameLoop.addSystem(std::make_shared<EventSystem>());
    // gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }
};

#endif
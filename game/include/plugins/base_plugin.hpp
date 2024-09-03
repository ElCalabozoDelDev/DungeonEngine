#ifndef BASE_PLUGIN_HPP
#define BASE_PLUGIN_HPP

#include "plugin.hpp"
#include <SDL.h>
#include <SDL_stdinc.h>
#include <entt/entt.hpp>
#include "core/game_loop.hpp"
#include "loaders/config.hpp"
#include "plugins/imgui_plugin.hpp"
#include "plugins/sdl_plugin.hpp"
class BasePlugin final : public Plugin {
public:

private:
  SDLPlugin m_sdl;
  ImGuiPlugin m_imgui;
public:
  BasePlugin(const Config &config) : m_sdl(config) {}

  void mount(GameLoop &gameLoop) override;
};

#endif
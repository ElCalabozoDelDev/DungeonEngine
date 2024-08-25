#ifndef GAME_HPP
#define GAME_HPP

#include "entt/entt.hpp"
#include "systems/movement_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/update_animation_system.hpp"
#include "systems/imgui_system.hpp"
#include "core/config.hpp"
#include <SDL_render.h>

class Game {
private:
  static Game *s_pInstance;
  std::string m_xmlGamePath;
  bool m_running;
  SDL_Window *m_gWindow;
  SDL_Renderer *m_gRenderer;
  entt::registry m_registry;
  int m_frameDelay;
  Game();
  ~Game();
  MovementSystem m_movementSystem;
  RenderSystem m_renderSystem;
  TransformSystem m_transformSystem;
  UpdateAnimationSystem m_updateAnimationSystem;
  ImGuiSystem m_imguiSystem;
  Config m_config;
  void handleEvents();
  void update();
  void render();
  void clean();
  bool isRunning() const { return m_running; }
  void createEntities();
public:
  static Game *Instance() {
    if (s_pInstance == nullptr) {
      s_pInstance = new Game();
    }
    return s_pInstance;
  }

  void init(const char* xmlGamePath);
  void run();
};

#endif // GAME_HPP

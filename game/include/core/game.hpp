#ifndef GAME_HPP
#define GAME_HPP

#include "entt/entt.hpp"
#include "systems/movement_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"

class Game {
private:
  static Game *s_pInstance;
  bool running;
  SDL_Window *gWindow;
  SDL_Renderer *gRenderer;
  entt::registry registry;
  int m_fps;
  int m_frameDelay;
  Game();
  ~Game();
  MovementSystem m_movementSystem;
  RenderSystem m_renderSystem;
  TransformSystem m_transformSystem;
  void handleEvents();
  void update();
  void render();
  void clean();
  bool isRunning() const { return running; }
  void createEntities();
public:
  static Game *Instance() {
    if (s_pInstance == nullptr) {
      s_pInstance = new Game();
    }
    return s_pInstance;
  }

  void init(const char *title, int xpos, int ypos, int width, int height,
            bool fullscreen, int fps, int frameDelay);
  void run();
};

#endif // GAME_HPP

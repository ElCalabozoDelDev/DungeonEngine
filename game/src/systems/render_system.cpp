#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "graphics/renderer.hpp"
#include "loaders/config.hpp"
#include <iostream>

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
  auto config = registry.ctx().get<Config>();

  auto &cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;
  
  // Obtener área de cámara visible
  AABB cameraView{
    static_cast<int>(cameraPos.getX() - config.screenWidth / 2.0f),  // Centrar la cámara en X
    static_cast<int>(cameraPos.getY() - config.screenHeight / 2.0f), // Centrar la cámara en Y
    config.screenWidth, config.screenHeight};
  Renderer::renderGraphics(registry, cameraView);
  Renderer::renderGUI(registry);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}
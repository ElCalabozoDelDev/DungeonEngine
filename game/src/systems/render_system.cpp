#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "graphics/renderer.hpp"
#include "loaders/config.hpp"

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
  const auto &view = registry.view<CameraComponent, TransformComponent>();
  auto cameraEntity = *view.begin();
  auto &camera = view.get<CameraComponent>(cameraEntity);
  auto &cameraPos = view.get<TransformComponent>(cameraEntity).position;
  
  // Obtener área de cámara visible
  AABB cameraView{
    static_cast<int>(cameraPos.getX() - camera.cameraWidth / 2.0f),  // Centrar la cámara en X
    static_cast<int>(cameraPos.getY() - camera.cameraHeight / 2.0f), // Centrar la cámara en Y
    camera.cameraWidth, camera.cameraHeight};
  Renderer::renderGraphics(registry, cameraView);
  Renderer::renderGUI(registry);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}
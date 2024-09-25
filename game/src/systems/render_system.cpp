#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "core/renderer.hpp"
#include "loaders/config.hpp"
#include <iostream>

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
  auto config = registry.ctx().get<Config>();
  
  // Obtener Quadtrees desde el contexto
  auto &tileQuadtree = registry.ctx().get<std::shared_ptr<TileQuadtree>>();
  auto &spriteQuadtree = registry.ctx().get<std::shared_ptr<SpriteQuadtree>>();
  auto &cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;
  
  // Obtener área de cámara visible
  AABB cameraView{
    static_cast<int>(cameraPos.getX() - config.screenWidth / 2.0f),  // Centrar la cámara en X
    static_cast<int>(cameraPos.getY() - config.screenHeight / 2.0f), // Centrar la cámara en Y
    config.screenWidth, config.screenHeight};

  std::vector<entt::entity> visibleTiles;
  tileQuadtree->query(cameraView, visibleTiles, registry);
  Renderer::renderTiles(registry, visibleTiles);

  std::vector<entt::entity> visibleSprites;
  spriteQuadtree->query(cameraView, visibleSprites, registry);
  Renderer::renderSprites(registry, visibleSprites);

  Renderer::renderGUI(registry);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}
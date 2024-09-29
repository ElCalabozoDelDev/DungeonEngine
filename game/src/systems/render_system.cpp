#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "core/renderer.hpp"
#include "loaders/config.hpp"
#include <iostream>

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
  auto config = registry.ctx().get<Config>();
  
  // Obtener Quadtrees desde el contexto
  auto& bottomQuadtree = registry.ctx().get<std::shared_ptr<BottomLayerQuadtree>>();
  auto& overlayQuadtree = registry.ctx().get<std::shared_ptr<OverlayLayerQuadtree>>();
  auto& collisionQuadtree = registry.ctx().get<std::shared_ptr<CollisionLayerQuadtree>>();
  auto &objectQuadtree = registry.ctx().get<std::shared_ptr<ObjectQuadtree>>();

  auto &cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;
  
  // Obtener área de cámara visible
  AABB cameraView{
    static_cast<int>(cameraPos.getX() - config.screenWidth / 2.0f),  // Centrar la cámara en X
    static_cast<int>(cameraPos.getY() - config.screenHeight / 2.0f), // Centrar la cámara en Y
    config.screenWidth, config.screenHeight};

  std::vector<entt::entity> visibleBottomTiles;
  bottomQuadtree->query(cameraView, visibleBottomTiles, registry);
  Renderer::renderTiles(registry, visibleBottomTiles);

  std::vector<entt::entity> visibleOverlayTiles;
  overlayQuadtree->query(cameraView, visibleOverlayTiles, registry);
  Renderer::renderTiles(registry, visibleOverlayTiles);

  std::vector<entt::entity> visibleSprites;
  objectQuadtree->query(cameraView, visibleSprites, registry);
  Renderer::renderSprites(registry, visibleSprites);

  std::vector<entt::entity> visibleCollisions;
  collisionQuadtree->query(cameraView, visibleCollisions, registry);
  Renderer::renderTiles(registry, visibleCollisions);


  Renderer::renderGUI(registry);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}
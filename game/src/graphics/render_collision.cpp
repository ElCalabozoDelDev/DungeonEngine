#include "graphics/render_collision.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"

void RenderCollision::draw(entt::registry& registry, AABB cameraView, float offsetX, float offsetY) {
    // Obtener Quadtrees desde el contexto
    auto &collisionQuadtree =  registry.ctx().get<std::vector<std::shared_ptr<Quadtree>>>()[LayerType::COLLISION];

    std::vector<entt::entity> visibleCollisions;
    collisionQuadtree->query(cameraView, visibleCollisions, registry);
    Renderer::renderTiles(registry, visibleCollisions, offsetX, offsetY);
}
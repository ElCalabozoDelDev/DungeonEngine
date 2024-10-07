#include "graphics/render_collision.hpp"
#include "graphics/renderer.hpp"

void RenderCollision::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &collisionQuadtree = registry.ctx().get<std::shared_ptr<CollisionLayerQuadtree>>();

    std::vector<entt::entity> visibleCollisions;
    collisionQuadtree->query(cameraView, visibleCollisions, registry);
    Renderer::renderTiles(registry, visibleCollisions);
}
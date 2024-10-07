#include "graphics/render_bottom.hpp"
#include "graphics/renderer.hpp"

void RenderBottom::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &bottomQuadtree = registry.ctx().get<std::shared_ptr<BottomLayerQuadtree>>();

    std::vector<entt::entity> visibleBottomTiles;
    bottomQuadtree->query(cameraView, visibleBottomTiles, registry);
    Renderer::renderTiles(registry, visibleBottomTiles);
}
#include "graphics/render_overlay.hpp"
#include "graphics/renderer.hpp"

void RenderOverlay::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &overlayQuadtree = registry.ctx().get<std::shared_ptr<OverlayLayerQuadtree>>();

    std::vector<entt::entity> visibleOverlayTiles;
    overlayQuadtree->query(cameraView, visibleOverlayTiles, registry);
    Renderer::renderTiles(registry, visibleOverlayTiles);
}
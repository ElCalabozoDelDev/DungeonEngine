#include "graphics/render_overlay.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"

void RenderOverlay::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &overlayQuadtree = registry.ctx().get<std::vector<std::shared_ptr<Quadtree>>>()[LayerType::OVERLAY];

    std::vector<entt::entity> visibleOverlayTiles;
    overlayQuadtree->query(cameraView, visibleOverlayTiles, registry);
    Renderer::renderTiles(registry, visibleOverlayTiles);
}
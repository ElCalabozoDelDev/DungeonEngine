#include "graphics/render_overlay.hpp"
#include "core/quadtree_manager.hpp"
#include "core/constants.hpp" // Add this line to include the definition of LayerType
#include "graphics/renderer.hpp" // Add this line to include the definition of Renderer

void RenderOverlay::draw(entt::registry &registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) {
    std::vector<entt::entity> visibleTiles = QuadtreeManager::Instance()->query(LayerType::OVERLAY, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}
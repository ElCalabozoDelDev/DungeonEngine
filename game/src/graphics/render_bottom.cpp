#include "graphics/render_bottom.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"
#include "core/quadtree_manager.hpp" // Include the header for QuadtreeManager

void RenderBottom::draw(entt::registry &registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) {
    std::vector<entt::entity> visibleTiles = QuadtreeManager::Instance()->query(LayerType::BOTTOM, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}
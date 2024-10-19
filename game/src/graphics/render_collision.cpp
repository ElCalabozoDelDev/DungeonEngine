#include "graphics/render_collision.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"
#include "core/quadtree_manager.hpp"

void RenderCollision::draw(entt::registry &registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) {
    std::vector<entt::entity> visibleTiles = QuadtreeManager::Instance()->query(LayerType::COLLISION, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}
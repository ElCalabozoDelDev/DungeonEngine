#include "graphics/render_object.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"
#include "core/quadtree_manager.hpp"

void RenderObject::draw(entt::registry &registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) {
    std::vector<entt::entity> visibleTiles = QuadtreeManager::Instance()->query(LayerType::OBJECT, cameraView);
    Renderer::renderSprites(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}
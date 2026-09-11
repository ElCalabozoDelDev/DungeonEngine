#include <engine/core/constants.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/quadtree_manager.hpp> // Include the header for QuadtreeManager

namespace de
{
void RenderBottom::draw(entt::registry& registry, Box<float> cameraView,
                        float offsetX, float offsetY, float zoomLevel)
{
    std::vector<entt::entity> visibleTiles =
        QuadtreeManager::Instance()->query(LayerType::BOTTOM, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}

} // namespace de

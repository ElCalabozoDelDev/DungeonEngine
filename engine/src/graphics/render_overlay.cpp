#include <engine/core/constants.hpp> // Add this line to include the definition of LayerType
#include <engine/graphics/render_overlay.hpp>
#include <engine/graphics/renderer.hpp> // Add this line to include the definition of Renderer
#include <engine/spatial/quadtree_manager.hpp>

namespace de
{
void RenderOverlay::draw(entt::registry& registry, Box<float> cameraView,
                         float offsetX, float offsetY, float zoomLevel)
{
    std::vector<entt::entity> visibleTiles =
        QuadtreeManager::Instance()->query(LayerType::OVERLAY, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}

} // namespace de

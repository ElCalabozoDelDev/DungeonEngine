#include <engine/core/constants.hpp>
#include <engine/graphics/render_collision.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/quadtree_manager.hpp>

namespace de
{
void RenderCollision::draw(entt::registry& registry, Box<float> cameraView,
                           float offsetX, float offsetY, float zoomLevel)
{
    std::vector<entt::entity> visibleTiles =
        QuadtreeManager::Instance()->query(LayerType::COLLISION, cameraView);
    Renderer::renderTiles(registry, visibleTiles, offsetX, offsetY, zoomLevel);
}

} // namespace de

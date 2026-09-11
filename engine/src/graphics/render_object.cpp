#include <engine/core/constants.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/quadtree_manager.hpp>

namespace de
{
void RenderObject::draw(entt::registry& registry, Box<float> cameraView,
                        float offsetX, float offsetY, float zoomLevel)
{
    std::vector<entt::entity> visibleTiles =
        QuadtreeManager::Instance()->query(LayerType::OBJECT, cameraView);
    Renderer::renderSprites(registry, visibleTiles, offsetX, offsetY,
                            zoomLevel);
}

} // namespace de

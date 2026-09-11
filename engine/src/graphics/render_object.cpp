#include <engine/graphics/render_object.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
void RenderObject::draw(entt::registry& registry, Box<float> cameraView,
                        float offsetX, float offsetY, float zoomLevel)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    std::vector<entt::entity> visible =
        spatial.query(Layer::Object, cameraView);
    Renderer::renderSprites(registry, visible, offsetX, offsetY, zoomLevel);
}

} // namespace de

#include <engine/graphics/render_overlay.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
void RenderOverlay::draw(entt::registry& registry, Box<float> cameraView,
                         float offsetX, float offsetY, float zoomLevel)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    std::vector<entt::entity> visible =
        spatial.query(Layer::Overlay, cameraView);
    Renderer::renderTiles(registry, visible, offsetX, offsetY, zoomLevel);
}

} // namespace de

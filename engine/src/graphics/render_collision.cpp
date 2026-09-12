#include <engine/graphics/render_collision.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
void RenderCollision::draw(entt::registry& registry, const Camera2D& camera)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    // One margin of a tile so entities straddling the edge still draw.
    const std::vector<entt::entity> visible =
        spatial.query(Layer::Collision, camera.visibleWorld(32.0f));
    Renderer::renderTiles(registry, visible, camera);
}

} // namespace de

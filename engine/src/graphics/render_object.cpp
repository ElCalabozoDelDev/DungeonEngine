#include <engine/graphics/render_object.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
void RenderObject::draw(entt::registry& registry, const Camera2D& camera)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    // One margin of a tile so entities straddling the edge still draw.
    const std::vector<entt::entity> visible =
        spatial.query(Layer::Object, camera.visibleWorld(32.0f));
    Renderer::renderSprites(registry, visible, camera);
}

} // namespace de

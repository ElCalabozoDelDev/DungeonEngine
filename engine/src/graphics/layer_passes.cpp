#include <engine/graphics/layer_passes.hpp>
#include <engine/graphics/renderer.hpp>

namespace de
{
void TileLayerPass::draw(entt::registry& registry, const Camera2D& camera)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    const auto visible =
        spatial.query(m_layer, camera.visibleWorld(m_cullMargin));
    drawTiles(registry, visible, camera);
}

void SpriteLayerPass::draw(entt::registry& registry, const Camera2D& camera)
{
    const auto& spatial = registry.ctx().get<SpatialIndex>();
    const auto visible =
        spatial.query(m_layer, camera.visibleWorld(m_cullMargin));
    drawSprites(registry, visible, camera);
}

} // namespace de

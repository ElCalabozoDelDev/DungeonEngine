#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/spatial/level_spatial_index.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
namespace
{
template <typename LayerTag>
void fileTileLayers(entt::registry& registry, SpatialIndex& spatial,
                    Layer layer)
{
    for (auto entity : registry.view<TileLayerComponent, LayerTag>())
    {
        for (auto tile : registry.get<TileLayerComponent>(entity).tileEntities)
        {
            spatial.insert(layer, tile);
        }
    }
}

} // namespace

void buildLevelSpatialIndex(entt::registry& registry, float mapWidth,
                            float mapHeight)
{
    SpatialIndex::BoxFn getBox = [&registry](const entt::entity& entity)
    {
        const auto& transform = registry.get<TransformComponent>(entity);
        const auto& dimension = registry.get<DimensionComponent>(entity);
        return Box<float>(transform.position.x, transform.position.y,
                          dimension.width, dimension.height);
    };

    const Box<float> bounds(0.0f, 0.0f, mapWidth, mapHeight);
    auto& spatial = registry.ctx().get<SpatialIndex>();
    spatial.create(Layer::Bottom, getBox, bounds);
    spatial.create(Layer::Overlay, getBox, bounds);
    spatial.create(Layer::Collision, getBox, bounds);
    spatial.create(Layer::Object, getBox, bounds);

    fileTileLayers<BottomLayerComponent>(registry, spatial, Layer::Bottom);
    fileTileLayers<OverlayLayerComponent>(registry, spatial, Layer::Overlay);

    for (auto entity : registry.view<SpriteComponent>())
    {
        spatial.insert(Layer::Object, entity);
    }
}

} // namespace de

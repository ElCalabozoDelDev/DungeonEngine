#ifndef DE_SPATIAL_LEVEL_SPATIAL_INDEX_HPP
#define DE_SPATIAL_LEVEL_SPATIAL_INDEX_HPP

#include <entt/entt.hpp>

namespace de
{
/// Builds the context's SpatialIndex for a loaded level `mapWidth` by
/// `mapHeight` world units: a tree per layer, bounded by the map, filing
/// entities by their TransformComponent and DimensionComponent.
///
/// Files every tile of the Bottom and Overlay tile layers, and every entity
/// with a SpriteComponent in the Object layer. Call it once the level's
/// entities exist; whatever is spawned later is inserted by whoever spawns it.
void buildLevelSpatialIndex(entt::registry& registry, float mapWidth,
                            float mapHeight);

} // namespace de

#endif // DE_SPATIAL_LEVEL_SPATIAL_INDEX_HPP

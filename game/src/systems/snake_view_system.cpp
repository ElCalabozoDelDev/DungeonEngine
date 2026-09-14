#include <engine/components/transform_component.hpp>
#include <engine/core/math.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/geometry.hpp>
#include <game/prefabs.hpp>
#include <game/systems/snake_view_system.hpp>

using namespace de;

namespace
{
/// Top-left corner of a sprite centred where `segment` is, `progress` of
/// the way through its stride.
Vector2D<float> spriteCorner(const SlimeSegment& segment, float progress)
{
    const auto center = lerp(segment.at, segment.to, progress);
    return Vector2D<float>(center.x - game::kSegmentSize * 0.5f,
                           center.y - game::kSegmentSize * 0.5f);
}

/// One sprite entity per body segment. Segment 0 is drawn by the head entity
/// itself, so it gets none: giving it one as well drew the head twice, one
/// sprite on top of the other.
void syncBodySprites(entt::registry& registry, SnakeComponent& snake)
{
    auto& sprites = snake.segmentEntities;
    const std::size_t bodyCount =
        snake.segments.empty() ? 0 : snake.segments.size() - 1;
    auto* spatial = registry.ctx().find<SpatialIndex>();
    const bool indexed =
        spatial != nullptr && spatial->get(Layer::Object) != nullptr;

    while (sprites.size() < bodyCount)
    {
        auto entity =
            game::prefab::makeSegmentSprite(registry, game::kSegmentSize);
        if (indexed)
        {
            spatial->insert(Layer::Object, entity);
        }
        sprites.push_back(entity);
    }

    while (sprites.size() > bodyCount)
    {
        auto entity = sprites.back();
        sprites.pop_back();
        if (indexed)
        {
            spatial->remove(Layer::Object, entity);
        }
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }

    for (std::size_t i = 0; i < bodyCount; ++i)
    {
        registry.get<TransformComponent>(sprites[i]).position =
            spriteCorner(snake.segments[i + 1], snake.movementProgress);
    }
}

} // namespace

void SnakeViewSystem::run(entt::registry& registry)
{
    for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
    {
        auto& snake = registry.get<SnakeComponent>(entity);
        syncBodySprites(registry, snake);

        if (!snake.segments.empty())
        {
            if (auto* transform = registry.try_get<TransformComponent>(entity))
            {
                transform->position = spriteCorner(snake.segments.front(),
                                                   snake.movementProgress);
            }
        }
    }
}

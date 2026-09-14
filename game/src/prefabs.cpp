#include <engine/components/animation_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/transform_component.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/geometry.hpp>
#include <game/prefabs.hpp>
#include <game/systems/bat_system.hpp>

using namespace de;

namespace game::prefab
{
namespace
{
void attachSprite(entt::registry& registry, entt::entity entity,
                  const SpriteStrip& strip, float size)
{
    registry.emplace_or_replace<DimensionComponent>(entity, size, size);
    registry.emplace_or_replace<TextureComponent>(entity, strip.texture);
    registry.emplace_or_replace<SpriteComponent>(entity, 0, 0, 0);
    AnimationComponent animation;
    animation.totalFrames = strip.frames;
    animation.animationTime = strip.frameSeconds;
    registry.emplace_or_replace<AnimationComponent>(entity, animation);
}

} // namespace

void makeSnakeHead(entt::registry& registry, entt::entity entity,
                   const Vector2D<float>& center, float size)
{
    SlimeSegment head;
    head.at = center;
    head.to = center;
    head.direction = Vector2D<float>(1.0f, 0.0f);

    SnakeComponent snake;
    snake.stride = size;
    snake.segments.push_back(head);
    snake.nextDirection = head.direction;

    registry.emplace_or_replace<PlayerComponent>(entity);
    registry.emplace_or_replace<SnakeComponent>(entity, snake);
    attachSprite(registry, entity, SlimeStrip, size);
    // The transform is the sprite's top-left corner.
    registry.emplace_or_replace<TransformComponent>(
        entity,
        Vector2D<float>(center.x - size * 0.5f, center.y - size * 0.5f));
}

entt::entity makeSegmentSprite(entt::registry& registry, float size)
{
    auto entity = registry.create();
    registry.emplace<TransformComponent>(entity, Vector2D<float>{});
    attachSprite(registry, entity, SlimeStrip, size);
    return entity;
}

entt::entity makeBat(entt::registry& registry, const Box<float>& room,
                     std::mt19937& rng)
{
    std::uniform_real_distribution<float> xDist(room.left(),
                                                room.right() - kSegmentSize);
    std::uniform_real_distribution<float> yDist(room.top(),
                                                room.bottom() - kSegmentSize);

    // Drawn in a fixed order -- heading, x, y -- so a seed gives the same bat
    // on every compiler. As two arguments of one call, the order of x and y
    // was whatever the compiler chose.
    BatComponent bat;
    bat.velocity = bat::randomVelocity(bat.speed, rng);
    const float x = xDist(rng);
    const float y = yDist(rng);

    auto entity = registry.create();
    registry.emplace<BatComponent>(entity, bat);
    registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
    attachSprite(registry, entity, BatStrip, kSegmentSize);
    // No VelocityComponent: BatSystem integrates against roomBounds itself.
    return entity;
}

} // namespace game::prefab

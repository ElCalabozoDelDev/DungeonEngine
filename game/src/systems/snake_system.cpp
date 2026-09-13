#include <algorithm>
#include <cmath>
#include <engine/components/animation_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/paused.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/state.hpp>
#include <game/systems/snake_system.hpp>
#include <vector>

using namespace de;

namespace
{
constexpr float SegmentSize = 20.0f;

float dot(const Vector2D<float>& a, const Vector2D<float>& b)
{
    return a.getX() * b.getX() + a.getY() * b.getY();
}

Vector2D<float> lerp(const Vector2D<float>& a, const Vector2D<float>& b,
                     float t)
{
    return a + (b - a) * t;
}

/// MonoGame-style AABB: sprite top-left at center - half size must stay inside
/// roomBounds (the open floor inset by one wall tile).
bool spriteOutsideRoom(const Vector2D<float>& center, float size,
                       const Rectangle& box)
{
    const float half = size * 0.5f;
    const float left = center.getX() - half;
    const float right = center.getX() + half;
    const float top = center.getY() - half;
    const float bottom = center.getY() + half;
    return left < box.left() || right > box.right() || top < box.top() ||
           bottom > box.bottom();
}

bool segmentsOverlap(const Vector2D<float>& a, const Vector2D<float>& b,
                     float size)
{
    const float half = size * 0.5f;
    const Box<float> aa(a.getX() - half, a.getY() - half, size, size);
    const Box<float> bb(b.getX() - half, b.getY() - half, size, size);
    return aa.intersects(bb);
}

void syncSegmentSprites(entt::registry& registry, SnakeComponent& snake,
                        std::vector<entt::entity>& segmentEntities)
{
    while (segmentEntities.size() < snake.segments.size())
    {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, Vector2D<float>{});
        registry.emplace<DimensionComponent>(entity, SegmentSize, SegmentSize);
        registry.emplace<SpriteComponent>(entity, 0, 0, 0);
        registry.emplace<TextureComponent>(entity, "slime");
        AnimationComponent anim;
        anim.totalFrames = 2;
        anim.animationTime = 0.2f;
        registry.emplace<AnimationComponent>(entity, anim);
        if (auto* spatial = registry.ctx().find<SpatialIndex>();
            spatial != nullptr && spatial->get(Layer::Object) != nullptr)
        {
            spatial->insert(Layer::Object, entity);
        }
        segmentEntities.push_back(entity);
    }

    while (segmentEntities.size() > snake.segments.size())
    {
        auto entity = segmentEntities.back();
        segmentEntities.pop_back();
        if (auto* spatial = registry.ctx().find<SpatialIndex>();
            spatial != nullptr && spatial->get(Layer::Object) != nullptr)
        {
            spatial->remove(Layer::Object, entity);
        }
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }

    for (std::size_t i = 0; i < snake.segments.size(); ++i)
    {
        const auto& segment = snake.segments[i];
        const auto pos = lerp(segment.at, segment.to, snake.movementProgress);
        auto& transform =
            registry.get<TransformComponent>(segmentEntities[i]).position;
        transform = Vector2D<float>(pos.getX() - SegmentSize * 0.5f,
                                    pos.getY() - SegmentSize * 0.5f);
    }
}

void syncPlayerTransform(entt::registry& registry, entt::entity entity,
                         const SnakeComponent& snake)
{
    if (snake.segments.empty() || !registry.all_of<TransformComponent>(entity))
    {
        return;
    }
    const auto& head = snake.segments.front();
    const auto pos = lerp(head.at, head.to, snake.movementProgress);
    registry.get<TransformComponent>(entity).position = Vector2D<float>(
        pos.getX() - SegmentSize * 0.5f, pos.getY() - SegmentSize * 0.5f);
}

} // namespace

void SnakeSystem::run(entt::registry& registry)
{
    auto* state = registry.ctx().find<GameState>();
    auto* paused = registry.ctx().find<Paused>();
    if (state == nullptr)
    {
        return;
    }

    // Freeze the final pose on Game Over so the head sits on the wall cell
    // instead of snapping back to the previous tile (which looked like an
    // early collision and made the overlay easy to miss).
    if (state->playState == PlayState::GameOver)
    {
        auto view = registry.view<PlayerComponent, SnakeComponent>();
        for (auto entity : view)
        {
            auto& snake = view.get<SnakeComponent>(entity);
            snake.movementProgress = 1.0f;
            syncSegmentSprites(registry, snake, snake.segmentEntities);
            syncPlayerTransform(registry, entity, snake);
        }
        return;
    }

    if (state->playState != PlayState::Playing)
    {
        return;
    }
    if (paused != nullptr && paused->value)
    {
        return;
    }

    auto* input = registry.ctx().find<InputState>();
    auto* actions = registry.ctx().find<ActionMap>();
    const float dt = registry.ctx().get<DeltaTime>().fixed;

    auto view = registry.view<PlayerComponent, SnakeComponent>();
    for (auto entity : view)
    {
        auto& snake = view.get<SnakeComponent>(entity);

        if (input != nullptr && actions != nullptr)
        {
            Vector2D<float> desired = snake.nextDirection;
            if (actions->isDown(*input, "move_up"))
            {
                desired = Vector2D<float>(0.0f, -1.0f);
            }
            else if (actions->isDown(*input, "move_down"))
            {
                desired = Vector2D<float>(0.0f, 1.0f);
            }
            else if (actions->isDown(*input, "move_left"))
            {
                desired = Vector2D<float>(-1.0f, 0.0f);
            }
            else if (actions->isDown(*input, "move_right"))
            {
                desired = Vector2D<float>(1.0f, 0.0f);
            }

            if (!snake.segments.empty())
            {
                const auto& facing = snake.segments.front().direction;
                if (dot(desired, facing) >= 0.0f)
                {
                    snake.nextDirection = desired;
                }
            }
            else
            {
                snake.nextDirection = desired;
            }
        }

        snake.movementTimer += dt;
        if (snake.movementTimer >= SnakeComponent::movementInterval)
        {
            snake.movementTimer -= SnakeComponent::movementInterval;

            if (snake.segments.empty())
            {
                continue;
            }

            const auto& head = snake.segments.front();
            SlimeSegment newHead;
            newHead.at = head.to;
            newHead.direction = snake.nextDirection;
            newHead.to = head.to + snake.nextDirection * snake.stride;

            snake.segments.insert(snake.segments.begin(), newHead);

            if (snake.pendingGrowth > 0)
            {
                --snake.pendingGrowth;
            }
            else
            {
                snake.segments.pop_back();
            }

            const auto& headPos = snake.segments.front().to;
            for (std::size_t i = 2; i < snake.segments.size(); ++i)
            {
                if (segmentsOverlap(headPos, snake.segments[i].to, SegmentSize))
                {
                    state->playState = PlayState::GameOver;
                    state->gameOver = true;
                    break;
                }
            }

            if (!state->gameOver &&
                spriteOutsideRoom(headPos, SegmentSize, state->roomBounds))
            {
                state->playState = PlayState::GameOver;
                state->gameOver = true;
            }

            if (state->gameOver)
            {
                // Show the head on the cell that killed it.
                snake.movementProgress = 1.0f;
                snake.movementTimer = 0.0f;
            }
        }

        if (!state->gameOver)
        {
            snake.movementProgress =
                snake.movementTimer / SnakeComponent::movementInterval;
        }

        syncSegmentSprites(registry, snake, snake.segmentEntities);
        syncPlayerTransform(registry, entity, snake);
    }
}

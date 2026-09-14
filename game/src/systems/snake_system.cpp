#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <game/components/player_component.hpp>
#include <game/geometry.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>
#include <game/systems/snake_system.hpp>

using namespace de;

namespace
{
/// MonoGame-style AABB: sprite top-left at center - half size must stay inside
/// roomBounds (the open floor inset by one wall tile).
bool spriteOutsideRoom(const Vector2D<float>& center, float size,
                       const Box<float>& box)
{
    const float half = size * 0.5f;
    const float left = center.x - half;
    const float right = center.x + half;
    const float top = center.y - half;
    const float bottom = center.y + half;
    return left < box.left() || right > box.right() || top < box.top() ||
           bottom > box.bottom();
}

bool segmentsOverlap(const Vector2D<float>& a, const Vector2D<float>& b,
                     float size)
{
    const float half = size * 0.5f;
    const Box<float> aa(a.x - half, a.y - half, size, size);
    const Box<float> bb(b.x - half, b.y - half, size, size);
    return aa.intersects(bb);
}

} // namespace

namespace game::snake
{
Vector2D<float> steer(const SnakeComponent& snake, const InputState& input,
                      const ActionMap& actions)
{
    Vector2D<float> desired = snake.nextDirection;
    if (actions.isDown(input, "move_up"))
    {
        desired = Vector2D<float>(0.0f, -1.0f);
    }
    else if (actions.isDown(input, "move_down"))
    {
        desired = Vector2D<float>(0.0f, 1.0f);
    }
    else if (actions.isDown(input, "move_left"))
    {
        desired = Vector2D<float>(-1.0f, 0.0f);
    }
    else if (actions.isDown(input, "move_right"))
    {
        desired = Vector2D<float>(1.0f, 0.0f);
    }

    // No 180-degree turns: the head would step straight onto the neck.
    if (!snake.segments.empty() &&
        desired.dot(snake.segments.front().direction) < 0.0f)
    {
        return snake.nextDirection;
    }
    return desired;
}

StepOutcome advance(SnakeComponent& snake, const Box<float>& room)
{
    const SlimeSegment head = snake.segments.front();
    const Vector2D<float> nextPos =
        head.to + snake.nextDirection * snake.stride;

    // Die on the last valid floor cell — do not step onto the wall.
    if (spriteOutsideRoom(nextPos, kSegmentSize, room))
    {
        snake.segments.front().at = head.to;
        snake.segments.front().to = head.to;
        return StepOutcome::HitWall;
    }

    SlimeSegment newHead;
    newHead.at = head.to;
    newHead.direction = snake.nextDirection;
    newHead.to = nextPos;
    snake.segments.insert(snake.segments.begin(), newHead);

    if (snake.pendingGrowth > 0)
    {
        --snake.pendingGrowth;
    }
    else
    {
        snake.segments.pop_back();
    }

    // Segment 1 is the neck, always adjacent to the head; only the rest can
    // be run into.
    const auto& headPos = snake.segments.front().to;
    for (std::size_t i = 2; i < snake.segments.size(); ++i)
    {
        if (segmentsOverlap(headPos, snake.segments[i].to, kSegmentSize))
        {
            return StepOutcome::HitSelf;
        }
    }
    return StepOutcome::Moved;
}

} // namespace game::snake

void SnakeSystem::run(entt::registry& registry)
{
    using game::snake::StepOutcome;

    // No pause or game-over check here: the loop does not step fixed systems
    // while de::Paused is set, and setPlayState() sets it for every state but
    // Playing. The step that ends the game settles the pose itself, below.
    auto& state = registry.ctx().get<GameState>();
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const float dt = registry.ctx().get<DeltaTime>().fixed;

    for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
    {
        auto& snake = registry.get<SnakeComponent>(entity);

        if (input != nullptr && actions != nullptr)
        {
            snake.nextDirection = game::snake::steer(snake, *input, *actions);
        }

        snake.movementTimer += dt;
        if (snake.movementTimer >= SnakeComponent::movementInterval)
        {
            snake.movementTimer -= SnakeComponent::movementInterval;

            if (snake.segments.empty())
            {
                continue;
            }

            if (game::snake::advance(snake, state.roomBounds) !=
                StepOutcome::Moved)
            {
                setPlayState(registry, PlayState::GameOver);
                if (auto* audio = registry.ctx().find<AudioManager>())
                {
                    audio->playSound("bounce");
                }
                // Freeze the pose where the stride ended.
                snake.movementProgress = 1.0f;
                snake.movementTimer = 0.0f;
            }
        }

        // Unless this step ended the game, which froze the pose above.
        if (state.playState == PlayState::Playing)
        {
            snake.movementProgress =
                snake.movementTimer / SnakeComponent::movementInterval;
        }
    }
}

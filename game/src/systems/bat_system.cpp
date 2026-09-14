#include <algorithm>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/math.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/geometry.hpp>
#include <game/rng.hpp>
#include <game/state.hpp>
#include <game/systems/bat_system.hpp>
#include <numbers>
#include <random>

using namespace de;

namespace
{
Circle<float> batCircle(const Vector2D<float>& topLeft, float width,
                        float height)
{
    // Tutorial Bat.GetBounds: radius = Width * 0.25.
    return Circle<float>{topLeft.getX() + width * 0.5f,
                         topLeft.getY() + height * 0.5f, width * 0.25f};
}

Circle<float> slimeHeadCircle(const SnakeComponent& snake)
{
    const auto& head = snake.segments.front();
    const auto pos = lerp(head.at, head.to, snake.movementProgress);
    // Tutorial Slime.GetBounds: radius = Width * 0.5 around the visual centre.
    return Circle<float>{pos.getX(), pos.getY(), game::kSegmentSize * 0.5f};
}

void bounce(BatComponent& bat, Vector2D<float>& position, float width,
            float height, Vector2D<float> normal, AudioManager* audio)
{
    if (normal.getX() != 0.0f)
    {
        position.setX(position.getX() + normal.getX() * (width * 0.1f));
    }
    if (normal.getY() != 0.0f)
    {
        position.setY(position.getY() + normal.getY() * (height * 0.1f));
    }
    bat.velocity = reflect(bat.velocity, normal);
    if (audio != nullptr)
    {
        audio->playSound("bounce");
    }
}

/// Place the bat on the opposite side of the room from the slime head
/// (MonoGame GameScene.PositionBatAwayFromSlime).
void positionBatAwayFromSlime(Vector2D<float>& batPos, float batW, float batH,
                              const SnakeComponent& snake,
                              const Box<float>& room, std::mt19937& rng)
{
    const float roomCenterX = room.getLeft() + room.getWidth() * 0.5f;
    const float roomCenterY = room.getTop() + room.getHeight() * 0.5f;
    const Circle<float> slime = slimeHeadCircle(snake);
    const float centerToSlimeX = slime.x - roomCenterX;
    const float centerToSlimeY = slime.y - roomCenterY;

    const float padding = std::max(batW, batH);

    if (std::abs(centerToSlimeX) > std::abs(centerToSlimeY))
    {
        std::uniform_int_distribution<int> yDist(
            static_cast<int>(room.getTop() + padding),
            static_cast<int>(room.getBottom() - padding));
        batPos.setY(static_cast<float>(yDist(rng)));
        if (centerToSlimeX > 0.0f)
        {
            batPos.setX(room.getLeft() + padding);
        }
        else
        {
            batPos.setX(room.getRight() - padding * 2.0f);
        }
    }
    else
    {
        std::uniform_int_distribution<int> xDist(
            static_cast<int>(room.getLeft() + padding),
            static_cast<int>(room.getRight() - padding));
        batPos.setX(static_cast<float>(xDist(rng)));
        if (centerToSlimeY > 0.0f)
        {
            batPos.setY(room.getTop() + padding);
        }
        else
        {
            batPos.setY(room.getBottom() - padding * 2.0f);
        }
    }
}

/// Moves the bat one fixed step and bounces it off the room walls.
void fly(BatComponent& bat, Vector2D<float>& position,
         const DimensionComponent& dimension, const Box<float>& room, float dt,
         std::mt19937& rng, AudioManager* audio)
{
    if (bat.velocity.lengthSquared() < 0.001f)
    {
        bat.velocity = game::bat::randomVelocity(bat.speed, rng);
    }

    // Integrate here (fixed dt). Do not use VelocityComponent — that would
    // let TransformSystem clamp against the full map instead of the room.
    position += bat.velocity * dt;

    const Circle<float> bounds =
        batCircle(position, dimension.width, dimension.height);
    Vector2D<float> normal(0.0f, 0.0f);

    if (bounds.left() < room.getLeft())
    {
        normal.setX(1.0f);
        position.setX(room.getLeft());
    }
    else if (bounds.right() > room.getRight())
    {
        normal.setX(-1.0f);
        position.setX(room.getRight() - dimension.width);
    }

    if (bounds.top() < room.getTop())
    {
        normal.setY(1.0f);
        position.setY(room.getTop());
    }
    else if (bounds.bottom() > room.getBottom())
    {
        normal.setY(-1.0f);
        position.setY(room.getBottom() - dimension.height);
    }

    if (normal.lengthSquared() > 0.0f)
    {
        bounce(bat, position, dimension.width, dimension.height, normal, audio);
    }
}

/// Lets every snake whose head touches the bat eat it: the snake grows, the
/// score goes up, and the bat reappears across the room with a new heading.
void feedTouchingSnakes(entt::registry& registry, GameState& state,
                        BatComponent& bat, Vector2D<float>& position,
                        const DimensionComponent& dimension, std::mt19937& rng,
                        AudioManager* audio)
{
    Circle<float> bounds =
        batCircle(position, dimension.width, dimension.height);

    for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
    {
        auto& snake = registry.get<SnakeComponent>(entity);
        if (snake.segments.empty() ||
            !slimeHeadCircle(snake).intersects(bounds))
        {
            continue;
        }

        ++snake.pendingGrowth;
        state.score += SnakeComponent::scorePerBat;
        if (audio != nullptr)
        {
            audio->playSound("collect");
        }

        positionBatAwayFromSlime(position, dimension.width, dimension.height,
                                 snake, state.roomBounds, rng);
        bat.velocity = game::bat::randomVelocity(bat.speed, rng);
        bounds = batCircle(position, dimension.width, dimension.height);
    }
}

} // namespace

namespace game::bat
{
Vector2D<float> randomVelocity(float speed, std::mt19937& rng)
{
    std::uniform_real_distribution<float> angleDist(
        0.0f, 2.0f * std::numbers::pi_v<float>);
    const float angle = angleDist(rng);
    return Vector2D<float>(std::cos(angle), std::sin(angle)) * speed;
}

} // namespace game::bat

void BatSystem::run(entt::registry& registry)
{
    // No pause check: the loop does not step fixed systems while de::Paused
    // is set. The one thing to guard against is the snake having died earlier
    // in this same step -- the bat must not be eaten by a dead snake.
    auto& state = registry.ctx().get<GameState>();
    if (state.playState != PlayState::Playing)
    {
        return;
    }

    const float dt = registry.ctx().get<DeltaTime>().fixed;
    auto* audio = registry.ctx().find<AudioManager>();
    auto& rng = registry.ctx().get<GameRng>().engine;

    auto bats =
        registry.view<BatComponent, TransformComponent, DimensionComponent>();
    for (auto entity : bats)
    {
        auto& bat = bats.get<BatComponent>(entity);
        auto& position = bats.get<TransformComponent>(entity).position;
        const auto& dimension = bats.get<DimensionComponent>(entity);

        fly(bat, position, dimension, state.roomBounds, dt, rng, audio);
        feedTouchingSnakes(registry, state, bat, position, dimension, rng,
                           audio);
    }
}

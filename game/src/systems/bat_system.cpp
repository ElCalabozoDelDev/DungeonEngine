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

void randomizeVelocity(BatComponent& bat, std::mt19937& rng)
{
    std::uniform_real_distribution<float> angleDist(
        0.0f, 2.0f * std::numbers::pi_v<float>);
    const float angle = angleDist(rng);
    bat.velocity =
        Vector2D<float>(std::cos(angle), std::sin(angle)) * bat.speed;
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

} // namespace

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

    const auto& dt = registry.ctx().get<DeltaTime>();
    auto* audio = registry.ctx().find<AudioManager>();
    auto& rng = registry.ctx().get<GameRng>().engine;

    auto snakes = registry.view<PlayerComponent, SnakeComponent>();
    auto bats =
        registry.view<BatComponent, TransformComponent, DimensionComponent>();

    for (auto bat : bats)
    {
        auto& batComp = bats.get<BatComponent>(bat);
        auto& transform = bats.get<TransformComponent>(bat).position;
        auto& dimension = bats.get<DimensionComponent>(bat);

        if (batComp.velocity.lengthSquared() < 0.001f)
        {
            randomizeVelocity(batComp, rng);
        }

        // Integrate here (fixed dt). Do not use VelocityComponent — that would
        // let TransformSystem clamp against the full map instead of the room.
        transform += batComp.velocity * dt.fixed;

        Circle<float> bounds =
            batCircle(transform, dimension.width, dimension.height);
        Vector2D<float> normal(0.0f, 0.0f);

        if (bounds.left() < state.roomBounds.getLeft())
        {
            normal.setX(1.0f);
            transform.setX(state.roomBounds.getLeft());
        }
        else if (bounds.right() > state.roomBounds.getRight())
        {
            normal.setX(-1.0f);
            transform.setX(state.roomBounds.getRight() - dimension.width);
        }

        if (bounds.top() < state.roomBounds.getTop())
        {
            normal.setY(1.0f);
            transform.setY(state.roomBounds.getTop());
        }
        else if (bounds.bottom() > state.roomBounds.getBottom())
        {
            normal.setY(-1.0f);
            transform.setY(state.roomBounds.getBottom() - dimension.height);
        }

        if (normal.lengthSquared() > 0.0f)
        {
            bounce(batComp, transform, dimension.width, dimension.height,
                   normal, audio);
        }

        bounds = batCircle(transform, dimension.width, dimension.height);

        for (auto snakeEntity : snakes)
        {
            auto& snake = snakes.get<SnakeComponent>(snakeEntity);
            if (snake.segments.empty())
            {
                continue;
            }

            if (!slimeHeadCircle(snake).intersects(bounds))
            {
                continue;
            }

            ++snake.pendingGrowth;
            state.score += SnakeComponent::scorePerBat;
            if (audio != nullptr)
            {
                audio->playSound("collect");
            }

            positionBatAwayFromSlime(transform, dimension.width,
                                     dimension.height, snake, state.roomBounds,
                                     rng);
            randomizeVelocity(batComp, rng);
            bounds = batCircle(transform, dimension.width, dimension.height);
        }
    }
}

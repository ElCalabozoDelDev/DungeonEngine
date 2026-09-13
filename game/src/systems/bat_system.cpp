#include <algorithm>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/paused.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/state.hpp>
#include <game/systems/bat_system.hpp>
#include <random>

using namespace de;

namespace
{
constexpr float HeadSize = 20.0f;

bool aabbOverlap(const Vector2D<float>& aPos, float aW, float aH,
                 const Vector2D<float>& bPos, float bW, float bH)
{
    return !(
        aPos.getX() + aW <= bPos.getX() || bPos.getX() + bW <= aPos.getX() ||
        aPos.getY() + aH <= bPos.getY() || bPos.getY() + bH <= aPos.getY());
}

Vector2D<float> randomPointInRoom(const Rectangle& room, float size,
                                  std::mt19937& rng)
{
    std::uniform_real_distribution<float> dx(room.left(), room.right() - size);
    std::uniform_real_distribution<float> dy(room.top(), room.bottom() - size);
    return Vector2D<float>(dx(rng), dy(rng));
}

bool nearSnake(const Vector2D<float>& pos, float size,
               const SnakeComponent& snake)
{
    for (const auto& segment : snake.segments)
    {
        const auto center = segment.to;
        const float half = HeadSize * 0.5f;
        const Vector2D<float> segTopLeft(center.getX() - half,
                                         center.getY() - half);
        if (aabbOverlap(pos, size, size, segTopLeft, HeadSize, HeadSize))
        {
            return true;
        }
    }
    return false;
}

} // namespace

void BatSystem::run(entt::registry& registry)
{
    auto* state = registry.ctx().find<GameState>();
    auto* paused = registry.ctx().find<Paused>();
    if (state == nullptr || state->playState != PlayState::Playing)
    {
        return;
    }
    if (paused != nullptr && paused->value)
    {
        return;
    }

    auto* audio = registry.ctx().find<AudioManager>();

    auto snakes = registry.view<PlayerComponent, SnakeComponent,
                                TransformComponent, DimensionComponent>();
    auto bats = registry.view<BatComponent, TransformComponent,
                              DimensionComponent, VelocityComponent>();

    for (auto bat : bats)
    {
        auto& transform = bats.get<TransformComponent>(bat).position;
        auto& velocity = bats.get<VelocityComponent>(bat).velocity;
        auto& dimension = bats.get<DimensionComponent>(bat);
        const auto& batComp = bats.get<BatComponent>(bat);

        // TransformSystem already integrated velocity this step; only bounce
        // and clamp against the room here.
        if (velocity.lengthSquared() < 0.001f)
        {
            velocity = Vector2D<float>(batComp.speed, batComp.speed);
        }

        bool bounced = false;
        if (transform.getX() < state->roomBounds.left())
        {
            transform.setX(state->roomBounds.left());
            velocity.setX(std::abs(velocity.getX()));
            bounced = true;
        }
        else if (transform.getX() + dimension.width > state->roomBounds.right())
        {
            transform.setX(state->roomBounds.right() - dimension.width);
            velocity.setX(-std::abs(velocity.getX()));
            bounced = true;
        }

        if (transform.getY() < state->roomBounds.top())
        {
            transform.setY(state->roomBounds.top());
            velocity.setY(std::abs(velocity.getY()));
            bounced = true;
        }
        else if (transform.getY() + dimension.height >
                 state->roomBounds.bottom())
        {
            transform.setY(state->roomBounds.bottom() - dimension.height);
            velocity.setY(-std::abs(velocity.getY()));
            bounced = true;
        }

        if (bounced && audio != nullptr)
        {
            audio->playSound("bounce");
        }

        for (auto snakeEntity : snakes)
        {
            auto& snake = snakes.get<SnakeComponent>(snakeEntity);
            const auto& headTransform =
                snakes.get<TransformComponent>(snakeEntity).position;
            const auto& headDim = snakes.get<DimensionComponent>(snakeEntity);

            if (!aabbOverlap(transform, dimension.width, dimension.height,
                             headTransform, headDim.width, headDim.height))
            {
                continue;
            }

            ++snake.pendingGrowth;
            state->score += SnakeComponent::scorePerBat;
            if (audio != nullptr)
            {
                audio->playSound("collect");
            }

            static std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; attempt < 32; ++attempt)
            {
                const auto candidate =
                    randomPointInRoom(state->roomBounds, dimension.width, rng);
                if (!nearSnake(candidate, dimension.width, snake))
                {
                    transform = candidate;
                    break;
                }
            }
        }
    }
}

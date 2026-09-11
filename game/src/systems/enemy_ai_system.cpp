#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/vector_2d.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/player_component.hpp>
#include <game/systems/enemy_ai_system.hpp>

using namespace de;

void EnemyAISystem::run(entt::registry& registry)
{
    auto players = registry.view<PlayerComponent, TransformComponent>();
    if (players.begin() == players.end())
    {
        return;
    }
    const auto playerPosition =
        players.get<TransformComponent>(*players.begin()).position;

    auto enemies =
        registry.view<EnemyComponent, TransformComponent, VelocityComponent>();

    for (auto entity : enemies)
    {
        const auto& enemy = enemies.get<EnemyComponent>(entity);
        const auto& transform = enemies.get<TransformComponent>(entity);
        auto& velocity = enemies.get<VelocityComponent>(entity);

        Vector2D<float> toPlayer = playerPosition - transform.position;
        const float distance = toPlayer.length();

        if (distance > enemy.chaseRange || distance <= 0.0f)
        {
            velocity.velocity = Vector2D<float>(0.0f, 0.0f);
            continue;
        }

        toPlayer.normalize();
        velocity.velocity = toPlayer * enemy.speed;
    }
}

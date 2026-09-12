#include <engine/audio/audio_manager.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/spatial/quadtree.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <game/components/attack_component.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/state.hpp>
#include <game/systems/combat_system.hpp>
#include <vector>

using namespace de;

namespace
{
constexpr float InvulnerabilitySeconds = 1.0f;

Box<float> boxOf(entt::registry& registry, entt::entity entity)
{
    const auto& transform = registry.get<TransformComponent>(entity);
    const auto& dimension = registry.get<DimensionComponent>(entity);
    return Box<float>(transform.position.getX(), transform.position.getY(),
                      dimension.width, dimension.height);
}

float distanceSq(const Vector2D<float>& a, const Vector2D<float>& b)
{
    const float dx = a.getX() - b.getX();
    const float dy = a.getY() - b.getY();
    return dx * dx + dy * dy;
}
} // namespace

void CombatSystem::run(entt::registry& registry)
{
    const float dt = registry.ctx().get<DeltaTime>().fixed;
    auto* state = registry.ctx().find<GameState>();
    auto* audio = registry.ctx().find<AudioManager>();
    auto* spatial = registry.ctx().find<SpatialIndex>();
    auto* input = registry.ctx().find<InputState>();
    auto* actions = registry.ctx().find<ActionMap>();

    auto players = registry.view<PlayerComponent, TransformComponent,
                                 DimensionComponent, HealthComponent>();

    for (auto player : players)
    {
        auto& health = players.get<HealthComponent>(player);
        health.invulnerabilityFor =
            std::max(0.0f, health.invulnerabilityFor - dt);

        const Box<float> playerBox = boxOf(registry, player);
        const auto& playerPos =
            registry.get<TransformComponent>(player).position;

        // --- Offensive attack (shape TBD: range check works for melee now,
        // projectiles can reuse the same damage / death path later) ---
        if (registry.all_of<AttackComponent>(player))
        {
            auto& attack = registry.get<AttackComponent>(player);
            attack.cooldownRemaining =
                std::max(0.0f, attack.cooldownRemaining - dt);

            const bool pressed = input != nullptr && actions != nullptr &&
                                 actions->wasPressed(*input, "attack");
            if (pressed && attack.cooldownRemaining <= 0.0f)
            {
                attack.cooldownRemaining = attack.cooldownSeconds;
                const float rangeSq = attack.range * attack.range;

                std::vector<entt::entity> slain;
                auto enemies =
                    registry.view<EnemyComponent, TransformComponent,
                                  DimensionComponent, HealthComponent>();
                for (auto enemy : enemies)
                {
                    const auto& enemyPos =
                        enemies.get<TransformComponent>(enemy).position;
                    if (distanceSq(playerPos, enemyPos) > rangeSq)
                    {
                        continue;
                    }
                    auto& enemyHealth = enemies.get<HealthComponent>(enemy);
                    enemyHealth.current -= attack.damage;
                    if (enemyHealth.current <= 0)
                    {
                        slain.push_back(enemy);
                    }
                }
                for (auto enemy : slain)
                {
                    if (spatial != nullptr)
                    {
                        spatial->remove(Layer::Object, enemy);
                    }
                    registry.destroy(enemy);
                }
            }
        }

        // --- Enemies hurt on contact ---
        auto enemies =
            registry
                .view<EnemyComponent, TransformComponent, DimensionComponent>();
        for (auto enemy : enemies)
        {
            if (!playerBox.intersects(boxOf(registry, enemy)))
            {
                continue;
            }
            if (health.invulnerabilityFor > 0.0f)
            {
                continue;
            }

            health.current -= enemies.get<EnemyComponent>(enemy).contactDamage;
            health.invulnerabilityFor = InvulnerabilitySeconds;
            if (audio != nullptr)
            {
                audio->playSound("hurt");
            }
            if (health.current <= 0 && state != nullptr)
            {
                health.current = 0;
                state->gameOver = true;
            }
            break; // one hit per cooldown, whoever got there first
        }

        // --- Score items are collected on contact ---
        std::vector<entt::entity> collected;
        auto items =
            registry
                .view<ItemComponent, TransformComponent, DimensionComponent>();
        for (auto item : items)
        {
            if (playerBox.intersects(boxOf(registry, item)))
            {
                collected.push_back(item);
            }
        }

        for (auto item : collected)
        {
            if (state != nullptr)
            {
                state->itemsCollected +=
                    registry.get<ItemComponent>(item).value;
            }
            if (audio != nullptr)
            {
                audio->playSound("pickup");
            }
            if (spatial != nullptr)
            {
                spatial->remove(Layer::Object, item);
            }
            registry.destroy(item);
        }
    }
}

#include <engine/audio/audio_manager.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/spatial/quadtree.hpp>
#include <engine/spatial/spatial_index.hpp>
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

} // namespace

void CombatSystem::run(entt::registry& registry)
{
    const float dt = registry.ctx().get<DeltaTime>().fixed;
    auto* state = registry.ctx().find<GameState>();
    auto* audio = registry.ctx().find<AudioManager>();
    auto* spatial = registry.ctx().find<SpatialIndex>();

    auto players = registry.view<PlayerComponent, TransformComponent,
                                 DimensionComponent, HealthComponent>();

    for (auto player : players)
    {
        auto& health = players.get<HealthComponent>(player);
        health.invulnerabilityFor =
            std::max(0.0f, health.invulnerabilityFor - dt);

        const Box<float> playerBox = boxOf(registry, player);

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

        // --- Items are collected on contact ---
        // Collected in two passes: destroying an entity while iterating the
        // view it came from invalidates the iteration.
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
            // Out of the index before out of the registry, or the next query
            // hands back a destroyed handle.
            if (spatial != nullptr)
            {
                spatial->remove(Layer::Object, item);
            }
            registry.destroy(item);
        }
    }
}

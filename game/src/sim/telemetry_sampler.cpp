#include <cmath>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/game_loop.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/sim/telemetry_sampler.hpp>
#include <game/state.hpp>

using namespace de;

namespace
{
/// Steps of near-zero movement, with nothing collected, before the run is
/// declared stuck. 600 is ten simulated seconds.
constexpr int WatchdogSteps = 600;
constexpr float WatchdogDistance = 8.0f;

de::Vector2D<float> centreOf(const TransformComponent& transform,
                             const DimensionComponent& dimension)
{
    return de::Vector2D<float>(
        transform.position.getX() + dimension.width * 0.5f,
        transform.position.getY() + dimension.height * 0.5f);
}

} // namespace

TelemetrySampler::TelemetrySampler(const SeekItemsPolicy& policy, int maxSteps)
    : m_policy(policy), m_maxSteps(maxSteps)
{
}

TelemetrySampler& TelemetrySampler::setWriter(TelemetryWriter* writer)
{
    m_writer = writer;
    return *this;
}

void TelemetrySampler::finish(entt::registry& registry, const char* outcome)
{
    if (m_finished)
    {
        return;
    }
    m_finished = true;
    m_summary.outcome = outcome;
    if (auto* flow = registry.ctx().find<ControlFlow>())
    {
        *flow = ControlFlow::Exit;
    }
}

void TelemetrySampler::run(entt::registry& registry)
{
    if (m_finished)
    {
        return;
    }

    auto players = registry.view<PlayerComponent, TransformComponent,
                                 DimensionComponent, HealthComponent>();
    if (players.begin() == players.end())
    {
        // The first frames are still the menu: the level is requested in a
        // setup callback and applied by SceneSystem mid-frame. No player yet
        // is normal, and not a row.
        return;
    }

    const entt::entity player = *players.begin();
    const auto& transform = players.get<TransformComponent>(player);
    const auto& dimension = players.get<DimensionComponent>(player);
    const auto& health = players.get<HealthComponent>(player);
    const auto* state = registry.ctx().find<GameState>();
    const auto& dt = registry.ctx().get<DeltaTime>();

    if (!m_started)
    {
        m_started = true;
        m_previousPosition = transform.position;
        m_previousHealth = health.current;
        m_summary.fixedDelta = dt.fixed;
        m_summary.itemsTotal = state != nullptr ? state->itemsTotal : 0;
        m_row = TelemetryRow{};
    }

    const float deltaX = transform.position.getX() - m_previousPosition.getX();
    const float deltaY = transform.position.getY() - m_previousPosition.getY();
    m_row.distance += std::sqrt(deltaX * deltaX + deltaY * deltaY);
    m_previousPosition = transform.position;

    m_row.step += 1;
    m_row.time = static_cast<float>(m_row.step) * dt.fixed;
    m_row.playerX = transform.position.getX();
    m_row.playerY = transform.position.getY();

    if (const auto* velocity = registry.try_get<VelocityComponent>(player))
    {
        m_row.velocityX = velocity->velocity.getX();
        m_row.velocityY = velocity->velocity.getY();
    }

    const PolicyCommand& command = m_policy.lastCommand();
    m_row.actionX = command.axisX;
    m_row.actionY = command.axisY;
    m_row.targetX = command.hasWaypoint ? command.waypoint.getX() : -1.0f;
    m_row.targetY = command.hasWaypoint ? command.waypoint.getY() : -1.0f;

    // An axis was asked for and collision zeroed it: the body is against
    // geometry. Measured after CollisionSystem, so this is the real outcome
    // of the step rather than the intent.
    const float stopped = 0.001f;
    m_row.blocked =
        (command.axisX != 0.0f && std::abs(m_row.velocityX) < stopped) ||
        (command.axisY != 0.0f && std::abs(m_row.velocityY) < stopped);

    m_row.health = health.current;
    m_row.invulnerable = health.invulnerabilityFor;
    if (m_previousHealth > health.current)
    {
        const int lost = m_previousHealth - health.current;
        m_row.damageTaken += lost;
        m_summary.hits += 1;
        if (m_summary.firstHitAt < 0.0f)
        {
            m_summary.firstHitAt = m_row.time;
        }
    }
    m_previousHealth = health.current;

    const int collected = state != nullptr ? state->itemsCollected : 0;
    for (int i = m_row.itemsCollected; i < collected; ++i)
    {
        m_summary.pickupTimes.push_back(m_row.time);
    }
    m_row.itemsCollected = collected;

    // Enemies: how many are in range, and how close the nearest is. Both are
    // about the chase-range tuning, which is the only enemy knob that changes
    // how the level plays.
    const de::Vector2D<float> playerCentre = centreOf(transform, dimension);
    m_row.enemiesChasing = 0;
    m_row.nearestEnemyDistance = -1.0f;
    for (auto enemy :
         registry
             .view<EnemyComponent, TransformComponent, DimensionComponent>())
    {
        const auto& enemyTransform = registry.get<TransformComponent>(enemy);
        const auto& enemyDimension = registry.get<DimensionComponent>(enemy);
        const de::Vector2D<float> enemyCentre =
            centreOf(enemyTransform, enemyDimension);
        const float dx = enemyCentre.getX() - playerCentre.getX();
        const float dy = enemyCentre.getY() - playerCentre.getY();
        const float distance = std::sqrt(dx * dx + dy * dy);

        if (m_row.nearestEnemyDistance < 0.0f ||
            distance < m_row.nearestEnemyDistance)
        {
            m_row.nearestEnemyDistance = distance;
        }
        if (distance <= registry.get<EnemyComponent>(enemy).chaseRange)
        {
            m_row.enemiesChasing += 1;
        }
    }

    m_row.gameOver = state != nullptr && state->gameOver;

    if (m_writer != nullptr)
    {
        m_writer->write(m_row);
    }

    m_summary.steps = m_row.step;
    m_summary.items = m_row.itemsCollected;
    m_summary.damageTaken = m_row.damageTaken;
    m_summary.finalHealth = m_row.health;
    m_summary.distance = m_row.distance;
    m_summary.unreachable = m_policy.unreachableItems();
    if (m_row.blocked)
    {
        m_summary.blockedSteps += 1;
    }
    if (m_row.enemiesChasing > 0)
    {
        m_summary.chasedSteps += 1;
    }
    if (m_row.nearestEnemyDistance >= 0.0f &&
        (m_summary.minEnemyDistance < 0.0f ||
         m_row.nearestEnemyDistance < m_summary.minEnemyDistance))
    {
        m_summary.minEnemyDistance = m_row.nearestEnemyDistance;
    }

    if (m_row.gameOver)
    {
        finish(registry, "died");
        return;
    }

    auto items = registry.view<ItemComponent>();
    const int itemsLeft = static_cast<int>(items.size());
    if (itemsLeft == 0)
    {
        finish(registry, "cleared");
        return;
    }
    if (m_policy.unreachableItems() >= itemsLeft)
    {
        // Everything still on the level is walled off from the player. The
        // run is over either way; saying so is more useful than burning the
        // remaining steps or calling it "cleared".
        finish(registry, "unreachable");
        return;
    }

    if (m_row.step >= m_maxSteps)
    {
        finish(registry, "step_cap");
        return;
    }

    // Watchdog: a silent 3600-step burn is worse than a loud failure.
    if (m_row.itemsCollected != m_watchdogItems ||
        m_row.distance - m_watchdogDistance >= WatchdogDistance)
    {
        m_watchdogItems = m_row.itemsCollected;
        m_watchdogDistance = m_row.distance;
        m_watchdogStep = m_row.step;
    }
    else if (m_row.step - m_watchdogStep >= WatchdogSteps)
    {
        finish(registry, "stuck");
    }
}

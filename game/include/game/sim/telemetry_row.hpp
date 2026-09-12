#ifndef GAME_SIM_TELEMETRY_ROW_HPP
#define GAME_SIM_TELEMETRY_ROW_HPP

#include <string>
#include <vector>

/// One fixed step of a simulated run.
///
/// Sampled after the whole fixed step, so it shows post-collision,
/// post-damage, post-pickup state -- what actually happened, not what was
/// about to. Every column is backed by something the game really has: there
/// is no player attack, no enemy health and no loot, so there is no DPS, no
/// kill count and no drop rate here either.
struct TelemetryRow
{
    int step = 0;
    /// step * DeltaTime::fixed. Simulated seconds, never wall clock.
    float time = 0.0f;

    float playerX = 0.0f;
    float playerY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    /// The policy asked for an axis and collision zeroed it: wall contact.
    bool blocked = false;
    /// Cumulative pixels travelled.
    float distance = 0.0f;

    int health = 0;
    /// Seconds of invulnerability left, so the 1 s cooldown is visible.
    float invulnerable = 0.0f;
    int damageTaken = 0;

    int itemsCollected = 0;
    /// Enemies within EnemyComponent::chaseRange -- the one number that
    /// measures how that range is tuned.
    int enemiesChasing = 0;
    /// Centre-to-centre; -1 when the level has no enemies at all.
    float nearestEnemyDistance = -1.0f;

    /// The waypoint being steered at, or (-1, -1) when idle. Records the
    /// decision, not just its outcome.
    float targetX = -1.0f;
    float targetY = -1.0f;
    float actionX = 0.0f;
    float actionY = 0.0f;

    bool gameOver = false;
};

/// Aggregates of a whole run, emitted once at teardown.
struct SimSummary
{
    /// cleared | died | unreachable | step_cap | stuck | interrupted |
    /// no_player
    std::string outcome = "no_player";
    int steps = 0;
    float fixedDelta = 0.0f;

    int items = 0;
    int itemsTotal = 0;
    int unreachable = 0;
    std::vector<float> pickupTimes;

    int damageTaken = 0;
    int hits = 0;
    int finalHealth = 0;
    float firstHitAt = -1.0f;

    float distance = 0.0f;
    int blockedSteps = 0;
    int chasedSteps = 0;
    float minEnemyDistance = -1.0f;

    unsigned int seed = 0;
    std::string csvPath;
};

#endif // GAME_SIM_TELEMETRY_ROW_HPP

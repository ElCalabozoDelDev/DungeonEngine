#ifndef GAME_SIM_SEEK_ITEMS_POLICY_HPP
#define GAME_SIM_SEEK_ITEMS_POLICY_HPP

#include <SDL.h>
#include <array>
#include <engine/core/vector_2d.hpp>
#include <entt/entt.hpp>
#include <game/sim/nav_grid.hpp>
#include <vector>

/// What the policy decided this frame, before it is turned into key presses.
/// Separated from applying it so the decision can be unit-tested without an
/// InputState.
struct PolicyCommand
{
    /// -1, 0 or +1, matching ActionMap::axis("move_left", "move_right").
    float axisX = 0.0f;
    /// -1, 0 or +1, matching ActionMap::axis("move_up", "move_down").
    float axisY = 0.0f;

    entt::entity target = entt::null;

    /// The tile-aligned point being steered at. Recorded in the telemetry so
    /// a run's decisions can be replayed, not just its outcomes.
    de::Vector2D<float> waypoint;
    bool hasWaypoint = false;
};

/// Drives the player towards the nearest remaining item, by path distance.
///
/// This is a repeatable probe, not a player. It does not dodge: it walks
/// through enemies and takes the hits, which is what produces the damage
/// signal. Any balance conclusion drawn from it is "how much does this level
/// punish someone mechanically perfect and tactically blind".
///
/// It navigates by breadth-first search over the static tile grid rather than
/// steering straight at the item. That is not sophistication for its own
/// sake: dungeon1's route passes a corridor exactly one tile wide, and a
/// greedy policy wedges against the wall and never reaches the second of
/// three items -- so the data would measure the policy, not the level.
class SeekItemsPolicy
{
public:
    SeekItemsPolicy& setSeed(unsigned int seed);

    /// Pure with respect to the world: works out what to press, changes
    /// nothing. Rebuilds the navigation grid when the level changes.
    PolicyCommand decide(entt::registry& registry);

    /// Writes the command into InputState as synthetic key presses, using the
    /// keys the game actually bound.
    void apply(entt::registry& registry, const PolicyCommand& command);

    void update(entt::registry& registry);

    /// Items with no reachable approach cell. Reported in the summary rather
    /// than silently stalling the run.
    int unreachableItems() const
    {
        return static_cast<int>(m_unreachable.size());
    }

    const PolicyCommand& lastCommand() const { return m_last; }

private:
    /// The best free, reachable cell overlapping an item's box.
    ///
    /// An item's own cell can be solid -- Coin3 in dungeon1 sits inside a
    /// wall -- so pathing to it directly never arrives.
    NavCell approachCell(const NavField& field,
                         const de::Vector2D<float>& topLeft, float width,
                         float height) const;

    NavGrid m_grid;
    entt::entity m_player = entt::null;
    unsigned int m_seed = 0;
    std::vector<entt::entity> m_unreachable;
    std::vector<entt::entity> m_order;
    PolicyCommand m_last;
    std::array<Uint8, SDL_NUM_SCANCODES> m_keys{};
};

#endif // GAME_SIM_SEEK_ITEMS_POLICY_HPP

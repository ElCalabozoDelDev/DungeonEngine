#include <algorithm>
#include <cmath>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/spatial/quadtree.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/sim/seek_items_policy.hpp>
#include <game/state.hpp>
#include <random>

using namespace de;

SeekItemsPolicy& SeekItemsPolicy::setSeed(unsigned int seed)
{
    m_seed = seed;
    return *this;
}

NavCell SeekItemsPolicy::approachCell(const NavField& field,
                                      const de::Vector2D<float>& topLeft,
                                      float width, float height) const
{
    // A cell is worth walking to only if standing there actually collects the
    // item -- CombatSystem picks it up on a strict AABB overlap, and touching
    // edges do not count.
    //
    // The player is exactly one tile, so a free cell and a standing body are
    // the same box: if no free cell's box overlaps the item, no position does
    // either, and the item cannot be collected by anyone. That is not
    // hypothetical -- Coin3 in dungeon1 sits entirely inside walls.
    const de::Box<float> itemBox(topLeft.getX(), topLeft.getY(), width, height);
    const float tile = m_grid.tileSize();

    const NavCell first = m_grid.cellAt(topLeft);
    const NavCell last = m_grid.cellAt(de::Vector2D<float>(
        topLeft.getX() + width - 1.0f, topLeft.getY() + height - 1.0f));

    NavCell reachableBest;
    int reachableDistance = -1;
    for (int row = first.row - 1; row <= last.row + 1; ++row)
    {
        for (int column = first.column - 1; column <= last.column + 1; ++column)
        {
            const NavCell cell{column, row};
            if (!field.reachable(cell))
            {
                continue;
            }
            const de::Vector2D<float> corner = m_grid.positionOf(cell);
            const de::Box<float> cellBox(corner.getX(), corner.getY(), tile,
                                         tile);
            if (!cellBox.intersects(itemBox))
            {
                continue;
            }
            const int distance = field.distance(cell);
            if (reachableDistance < 0 || distance < reachableDistance)
            {
                reachableDistance = distance;
                reachableBest = cell;
            }
        }
    }
    return reachableBest;
}

PolicyCommand SeekItemsPolicy::decide(entt::registry& registry)
{
    PolicyCommand command;

    auto players = registry.view<PlayerComponent, TransformComponent,
                                 DimensionComponent, SpeedComponent>();
    if (players.begin() == players.end())
    {
        m_player = entt::null;
        m_last = command;
        return command;
    }

    const entt::entity player = *players.begin();
    if (player != m_player)
    {
        // A different player entity means a fresh level (start, F5, replay),
        // so the static grid and the give-up list are rebuilt.
        m_grid.build(registry);
        m_player = player;
        m_unreachable.clear();
        m_order.clear();
    }
    if (m_grid.empty())
    {
        m_last = command;
        return command;
    }

    const auto* state = registry.ctx().find<GameState>();
    if (state != nullptr && state->gameOver)
    {
        m_last = command;
        return command;
    }

    const auto& transform = players.get<TransformComponent>(player);
    const auto& dimension = players.get<DimensionComponent>(player);
    const auto& speed = players.get<SpeedComponent>(player);
    const Vector2D<float> topLeft = transform.position;
    const Vector2D<float> centre(topLeft.getX() + dimension.width * 0.5f,
                                 topLeft.getY() + dimension.height * 0.5f);

    // Flood from the player. If the centre cell is solid -- possible for a
    // step while being pushed out of a wall -- fall back to a corner the body
    // also overlaps.
    NavCell from = m_grid.cellAt(centre);
    if (m_grid.isSolid(from))
    {
        const NavCell corners[] = {
            m_grid.cellAt(topLeft),
            m_grid.cellAt(Vector2D<float>(
                topLeft.getX() + dimension.width - 1.0f, topLeft.getY())),
            m_grid.cellAt(Vector2D<float>(
                topLeft.getX(), topLeft.getY() + dimension.height - 1.0f)),
            m_grid.cellAt(
                Vector2D<float>(topLeft.getX() + dimension.width - 1.0f,
                                topLeft.getY() + dimension.height - 1.0f)),
        };
        for (const NavCell& cell : corners)
        {
            if (!m_grid.isSolid(cell))
            {
                from = cell;
                break;
            }
        }
    }

    const NavField field = m_grid.flood(from);

    // Visiting order. Seed 0 means strict nearest-first; any other seed
    // shuffles, so a different route through the level can be measured.
    auto items =
        registry.view<ItemComponent, TransformComponent, DimensionComponent>();
    std::vector<entt::entity> remaining;
    for (auto item : items)
    {
        if (std::find(m_unreachable.begin(), m_unreachable.end(), item) ==
            m_unreachable.end())
        {
            remaining.push_back(item);
        }
    }
    if (remaining.empty())
    {
        m_last = command;
        return command;
    }
    // Sorted by id first so the order never depends on view iteration.
    std::sort(remaining.begin(), remaining.end(),
              [](entt::entity a, entt::entity b)
              { return entt::to_integral(a) < entt::to_integral(b); });

    entt::entity target = entt::null;
    NavCell goal;
    if (m_seed != 0)
    {
        if (m_order.empty())
        {
            m_order = remaining;
            std::mt19937 generator(m_seed);
            std::shuffle(m_order.begin(), m_order.end(), generator);
        }
        for (auto candidate : m_order)
        {
            if (std::find(remaining.begin(), remaining.end(), candidate) ==
                remaining.end())
            {
                continue;
            }
            const auto& itemTransform =
                items.get<TransformComponent>(candidate);
            const auto& itemDimension =
                items.get<DimensionComponent>(candidate);
            const NavCell cell =
                approachCell(field, itemTransform.position, itemDimension.width,
                             itemDimension.height);
            if (field.reachable(cell))
            {
                target = candidate;
                goal = cell;
                break;
            }
            m_unreachable.push_back(candidate);
        }
    }
    else
    {
        int best = -1;
        for (auto candidate : remaining)
        {
            const auto& itemTransform =
                items.get<TransformComponent>(candidate);
            const auto& itemDimension =
                items.get<DimensionComponent>(candidate);
            const NavCell cell =
                approachCell(field, itemTransform.position, itemDimension.width,
                             itemDimension.height);
            if (!field.reachable(cell))
            {
                continue;
            }
            const int distance = field.distance(cell);
            if (best < 0 || distance < best)
            {
                best = distance;
                target = candidate;
                goal = cell;
            }
        }
        if (target == entt::null)
        {
            // Nothing left that can be reached from here.
            for (auto candidate : remaining)
            {
                m_unreachable.push_back(candidate);
            }
        }
    }

    if (target == entt::null)
    {
        m_last = command;
        return command;
    }

    const NavCell next = field.stepTowards(goal);
    const Vector2D<float> waypoint = m_grid.positionOf(next);
    command.target = target;
    command.waypoint = waypoint;
    command.hasWaypoint = true;

    // Deadzone of half a step, derived rather than hardcoded so it stays
    // correct if SpeedComponent or DeltaTime::fixed is retuned. A step that
    // would overshoot the waypoint lands inside it, so this terminates.
    const auto& dt = registry.ctx().get<DeltaTime>();
    const float deadzone = 0.5f * speed.value * dt.fixed;

    const float deltaX = waypoint.getX() - topLeft.getX();
    const float deltaY = waypoint.getY() - topLeft.getY();

    // One axis at a time: pressing both would make the speed the diagonal
    // 1/sqrt(2) of what SpeedComponent says.
    //
    // Which axis, though, is the whole game. The waypoint is one tile away on
    // the travel axis and tile-aligned on the other, so the smaller of the
    // two deltas is the body's misalignment within its lane. Correct that
    // first: the player starts at x=182 in dungeon1, six pixels off the grid,
    // and a body one tile wide that is six pixels off overlaps the next
    // column. Travelling first means shoving that overhang into the wall
    // forever -- measured, before this rule existed: eleven pixels of
    // progress in ten simulated seconds, blocked half the steps.
    const bool moveX = std::abs(deltaX) > deadzone;
    const bool moveY = std::abs(deltaY) > deadzone;
    const bool alignXFirst =
        moveX && moveY ? std::abs(deltaX) <= std::abs(deltaY) : moveX;
    if (alignXFirst)
    {
        command.axisX = deltaX > 0.0f ? 1.0f : -1.0f;
    }
    else if (moveY)
    {
        command.axisY = deltaY > 0.0f ? 1.0f : -1.0f;
    }

    m_last = command;
    return command;
}

void SeekItemsPolicy::apply(entt::registry& registry,
                            const PolicyCommand& command)
{
    auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input == nullptr || actions == nullptr)
    {
        return;
    }

    m_keys.fill(Uint8{0});

    // The keys the game bound, not a hardcoded WASD that silently stops
    // matching the moment game_plugin.cpp changes a binding.
    const auto press = [&](const char* action)
    {
        const auto keys = actions->keysFor(action);
        if (!keys.empty())
        {
            m_keys[static_cast<std::size_t>(keys.front())] = 1;
        }
    };

    if (command.axisX < 0.0f)
    {
        press("move_left");
    }
    else if (command.axisX > 0.0f)
    {
        press("move_right");
    }
    if (command.axisY < 0.0f)
    {
        press("move_up");
    }
    else if (command.axisY > 0.0f)
    {
        press("move_down");
    }

    // Never pause, confirm or reload: those would take the run somewhere the
    // telemetry cannot describe.
    input->setKeyboard(m_keys.data(), SDL_NUM_SCANCODES);
    input->setCaptured(false, false);
}

void SeekItemsPolicy::update(entt::registry& registry)
{
    apply(registry, decide(registry));
}

#include <SDL.h>
#include <doctest/doctest.h>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/sim/seek_items_policy.hpp>
#include <game/state.hpp>

using namespace de;

namespace
{
constexpr float Tile = 16.0f;

/// A level made of tiles, a player and items -- everything the policy reads,
/// and nothing else. No window, no TMX, no game loop.
struct PolicyWorld
{
    entt::registry registry;
    entt::entity collisionLayer;

    PolicyWorld()
    {
        registry.ctx().emplace<DeltaTime>(
            DeltaTime{1.0f / 60.0f, 1.0f / 60.0f, 0.0f, 0.0});
        registry.ctx().emplace<GameState>();
        registry.ctx().emplace<InputState>();

        // The real bindings, so a rename in game_plugin.cpp shows up here.
        auto& actions = registry.ctx().emplace<ActionMap>();
        actions.bind("move_up", SDL_SCANCODE_UP);
        actions.bind("move_down", SDL_SCANCODE_DOWN);
        actions.bind("move_left", SDL_SCANCODE_LEFT);
        actions.bind("move_right", SDL_SCANCODE_RIGHT);

        collisionLayer = registry.create();
        registry.emplace<TileLayerComponent>(collisionLayer);
        registry.emplace<CollisionLayerComponent>(collisionLayer);
    }

    void wall(int column, int row)
    {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(
            entity, Vector2D<float>(static_cast<float>(column) * Tile,
                                    static_cast<float>(row) * Tile));
        registry.emplace<DimensionComponent>(entity, Tile, Tile);
        registry.get<TileLayerComponent>(collisionLayer)
            .tileEntities.push_back(entity);
    }

    /// Walls the whole border, so the grid has a known extent.
    void border(int columns, int rows)
    {
        for (int column = 0; column < columns; ++column)
        {
            wall(column, 0);
            wall(column, rows - 1);
        }
        for (int row = 0; row < rows; ++row)
        {
            wall(0, row);
            wall(columns - 1, row);
        }
    }

    entt::entity player(float x, float y)
    {
        auto entity = registry.create();
        registry.emplace<PlayerComponent>(entity);
        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<DimensionComponent>(entity, Tile, Tile);
        registry.emplace<SpeedComponent>(entity);
        return entity;
    }

    entt::entity item(float x, float y)
    {
        auto entity = registry.create();
        registry.emplace<ItemComponent>(entity);
        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<DimensionComponent>(entity, Tile, Tile);
        return entity;
    }

    /// Moves the player the way MovementSystem would, one fixed step, with no
    /// collision: enough to check that a command actually makes progress.
    void step(entt::entity who, const PolicyCommand& command)
    {
        auto& position = registry.get<TransformComponent>(who).position;
        const float speed = registry.get<SpeedComponent>(who).value / 60.0f;
        position.setX(position.getX() + command.axisX * speed);
        position.setY(position.getY() + command.axisY * speed);
    }
};

} // namespace

TEST_CASE("the policy steers at the nearest item and stops when none are left")
{
    PolicyWorld world;
    world.border(10, 10);
    auto player = world.player(5 * Tile, 5 * Tile);
    auto near = world.item(2 * Tile, 5 * Tile);
    world.item(8 * Tile, 5 * Tile);

    SeekItemsPolicy policy;
    const PolicyCommand command = policy.decide(world.registry);

    CHECK(command.target == near);
    CHECK(command.axisX == doctest::Approx(-1.0f));
    CHECK(command.axisY == doctest::Approx(0.0f));

    SUBCASE("one axis at a time, so the speed is never the diagonal")
    {
        CHECK((command.axisX == 0.0f) != (command.axisY == 0.0f));
    }

    SUBCASE("it retargets once the nearest item is collected")
    {
        world.registry.destroy(near);
        CHECK(policy.decide(world.registry).axisX == doctest::Approx(1.0f));
    }

    SUBCASE("no items left means no input at all")
    {
        world.registry.clear<ItemComponent>();
        const PolicyCommand idle = policy.decide(world.registry);
        CHECK(idle.axisX == doctest::Approx(0.0f));
        CHECK(idle.axisY == doctest::Approx(0.0f));
        CHECK((idle.target == entt::null));
    }

    SUBCASE("game over means no input at all")
    {
        world.registry.ctx().get<GameState>().gameOver = true;
        const PolicyCommand dead = policy.decide(world.registry);
        CHECK(dead.axisX == doctest::Approx(0.0f));
        CHECK(dead.axisY == doctest::Approx(0.0f));
    }

    SUBCASE("no player means no input at all")
    {
        world.registry.destroy(player);
        const PolicyCommand none = policy.decide(world.registry);
        CHECK((none.target == entt::null));
        CHECK(none.hasWaypoint == false);
    }
}

TEST_CASE("an item inside a wall is judged by whether it can be collected")
{
    // CombatSystem picks an item up on a strict AABB overlap, and the player
    // is exactly one tile. So standing in the free cell *next to* a walled-in
    // item does not collect it: the boxes only touch.
    //
    // This is not hypothetical. Coin3 in dungeon1 sits at (300,400), entirely
    // within solid cells, and no player -- human or scripted -- can pick it
    // up. Pathing "as close as possible" would make the run walk there and
    // stall; knowing the difference is what lets it report the level as
    // uncompletable instead.
    SeekItemsPolicy policy;

    SUBCASE("fully buried: no position overlaps it, so it is unreachable")
    {
        PolicyWorld world;
        world.border(10, 10);
        for (int column = 2; column <= 6; ++column)
        {
            world.wall(column, 5);
        }
        world.player(5 * Tile, 3 * Tile);
        world.item(4 * Tile, 5 * Tile); // exactly the solid cell (4,5)

        const PolicyCommand command = policy.decide(world.registry);
        CHECK((command.target == entt::null));
        CHECK(policy.unreachableItems() == 1);
    }

    SUBCASE("half buried: it sticks out into a free cell, so it is collected")
    {
        PolicyWorld world;
        world.border(10, 10);
        for (int column = 2; column <= 6; ++column)
        {
            world.wall(column, 5);
        }
        world.player(5 * Tile, 3 * Tile);
        // Eight pixels up: its box now overlaps free row 4 as well.
        auto reachable = world.item(4 * Tile, 5 * Tile - 8.0f);

        const PolicyCommand command = policy.decide(world.registry);
        CHECK(command.target == reachable);
        CHECK(policy.unreachableItems() == 0);

        REQUIRE(command.hasWaypoint);
        // It aims at the free row above, never into the wall.
        CHECK(command.waypoint.getY() <= 4 * Tile);
    }
}

TEST_CASE("the policy walks a one-tile corridor without wedging")
{
    // The Coin1 -> Coin2 leg of dungeon1 passes a corridor exactly one tile
    // wide. The body is exactly one tile too, so the approach has to be
    // tile-aligned or it jams against the wall forever.
    PolicyWorld world;
    world.border(9, 12);
    // A full wall across row 5, with a single gap at column 4.
    for (int column = 1; column <= 7; ++column)
    {
        if (column != 4)
        {
            world.wall(column, 5);
        }
    }
    auto player = world.player(2 * Tile, 8 * Tile);
    world.item(6 * Tile, 2 * Tile);

    SeekItemsPolicy policy;

    // Drive it for a while and check it actually gets through the gap rather
    // than grinding against the wall.
    bool passed = false;
    for (int step = 0; step < 400 && !passed; ++step)
    {
        const PolicyCommand command = policy.decide(world.registry);
        REQUIRE_FALSE((command.target == entt::null));
        world.step(player, command);
        const float y =
            world.registry.get<TransformComponent>(player).position.getY();
        passed = y < 5 * Tile;
    }
    CHECK(passed);

    SUBCASE("the waypoints it aims at are tile-aligned")
    {
        // This alignment is the whole mechanism: it leaves the residual error
        // under one step, so the AABB resolver snaps the body into the gap
        // instead of jamming it against the edge.
        const PolicyCommand command = policy.decide(world.registry);
        REQUIRE(command.hasWaypoint);
        CHECK(std::fmod(command.waypoint.getX(), Tile) ==
              doctest::Approx(0.0f));
        CHECK(std::fmod(command.waypoint.getY(), Tile) ==
              doctest::Approx(0.0f));
    }
}

TEST_CASE("an item sealed off is given up on, not chased forever")
{
    PolicyWorld world;
    world.border(12, 8);
    for (int row = 1; row < 7; ++row)
    {
        world.wall(6, row);
    }
    world.player(2 * Tile, 3 * Tile);
    auto sealed = world.item(9 * Tile, 3 * Tile);
    auto open = world.item(3 * Tile, 5 * Tile);

    SeekItemsPolicy policy;
    const PolicyCommand command = policy.decide(world.registry);

    CHECK(command.target == open);

    SUBCASE("the unreachable one is reported, not silently skipped")
    {
        world.registry.destroy(open);
        policy.decide(world.registry);
        CHECK(policy.unreachableItems() == 1);
        CHECK(world.registry.valid(sealed));
    }
}

TEST_CASE("apply writes keys that ActionMap reads back as the command")
{
    // The integration point most likely to rot: the policy presses scancodes,
    // MovementSystem asks ActionMap for axes, and nothing connects the two
    // except the binding names.
    PolicyWorld world;
    world.border(10, 10);
    world.player(5 * Tile, 5 * Tile);
    world.item(2 * Tile, 5 * Tile);

    SeekItemsPolicy policy;
    const PolicyCommand command = policy.decide(world.registry);
    policy.apply(world.registry, command);

    const auto& input = world.registry.ctx().get<InputState>();
    const auto& actions = world.registry.ctx().get<ActionMap>();

    CHECK(actions.axis(input, "move_left", "move_right") ==
          doctest::Approx(command.axisX));
    CHECK(actions.axis(input, "move_up", "move_down") ==
          doctest::Approx(command.axisY));

    SUBCASE("it never presses pause, confirm or reload")
    {
        CHECK_FALSE(input.isDown(SDL_SCANCODE_ESCAPE));
        CHECK_FALSE(input.isDown(SDL_SCANCODE_RETURN));
        CHECK_FALSE(input.isDown(SDL_SCANCODE_F5));
    }

    SUBCASE("an idle command releases everything")
    {
        policy.apply(world.registry, PolicyCommand{});
        const auto& idle = world.registry.ctx().get<InputState>();
        CHECK(actions.axis(idle, "move_left", "move_right") ==
              doctest::Approx(0.0f));
        CHECK(actions.axis(idle, "move_up", "move_down") ==
              doctest::Approx(0.0f));
    }
}

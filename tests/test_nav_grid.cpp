#include <doctest/doctest.h>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <game/sim/nav_grid.hpp>

using namespace de;

namespace
{
/// A registry holding only what NavGrid::build reads: tile entities under a
/// layer tagged Collision or Overlay. No window, no loader, no level file.
struct TileWorld
{
    entt::registry registry;
    float tileSize = 16.0f;

    entt::entity layer(bool overlay)
    {
        auto entity = registry.create();
        registry.emplace<TileLayerComponent>(entity);
        if (overlay)
        {
            registry.emplace<OverlayLayerComponent>(entity);
        }
        else
        {
            registry.emplace<CollisionLayerComponent>(entity);
        }
        return entity;
    }

    void tile(entt::entity layerEntity, int column, int row)
    {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(
            entity, Vector2D<float>(static_cast<float>(column) * tileSize,
                                    static_cast<float>(row) * tileSize));
        registry.emplace<DimensionComponent>(entity, tileSize, tileSize);
        registry.get<TileLayerComponent>(layerEntity)
            .tileEntities.push_back(entity);
    }
};

} // namespace

TEST_CASE("the grid is built from both solid layers")
{
    TileWorld world;
    auto collision = world.layer(false);
    auto overlay = world.layer(true);
    world.tile(collision, 1, 0);
    world.tile(overlay, 3, 2);

    NavGrid grid;
    grid.build(world.registry);

    CHECK_FALSE(grid.empty());
    CHECK(grid.tileSize() == doctest::Approx(16.0f));

    SUBCASE("Overlay is solid too, not just Collision")
    {
        // CollisionSystem treats both as solid (SolidLayers). A grid that
        // only read Collision would route the player straight through a wall.
        CHECK(grid.isSolid(NavCell{1, 0}));
        CHECK(grid.isSolid(NavCell{3, 2}));
        CHECK_FALSE(grid.isSolid(NavCell{2, 1}));
    }
}

TEST_CASE("positions and cells convert both ways")
{
    NavGrid grid;
    grid.reset(10, 10, 16.0f);

    SUBCASE("a tile-aligned position round-trips exactly")
    {
        const NavCell cell{3, 4};
        const auto position = grid.positionOf(cell);
        CHECK(position.getX() == doctest::Approx(48.0f));
        CHECK(position.getY() == doctest::Approx(64.0f));
        CHECK(grid.cellAt(position) == cell);
    }

    SUBCASE("a misaligned position lands in the cell containing it")
    {
        // The player is not grid-aligned -- it starts at x=182 in dungeon1.
        CHECK(grid.cellAt(Vector2D<float>(190.0f, 60.0f)) == NavCell{11, 3});
    }

    SUBCASE("out of bounds is solid, so nobody walks off the map")
    {
        CHECK(grid.isSolid(NavCell{-1, 0}));
        CHECK(grid.isSolid(NavCell{10, 0}));
        CHECK_FALSE(grid.inBounds(NavCell{0, 10}));
    }
}

TEST_CASE("the flood finds paths around walls")
{
    // A wall down column 2, with a gap at row 4:
    //   . . # . .
    //   . . # . .
    //   . . . . .   <- row 4, the only way through
    NavGrid grid;
    grid.reset(5, 5, 16.0f);
    for (int row = 0; row < 4; ++row)
    {
        grid.setSolid(2, row, true);
    }

    const NavField field = grid.flood(NavCell{0, 0});

    SUBCASE("distance counts the detour, not the straight line")
    {
        CHECK(field.distance(NavCell{0, 0}) == 0);
        // 4 down, 2 right, 4 up -- not the 4 of a straight line.
        CHECK(field.distance(NavCell{4, 0}) == 4 + 4 + 4);
    }

    SUBCASE("following the steps walks the whole path, never into the wall")
    {
        // Not "the step is (0,1)": with several shortest paths the neighbour
        // order decides which one, and pinning that pins an implementation
        // detail. What has to hold is that repeatedly taking the step
        // arrives, in exactly the distance reported, over free cells only.
        const NavCell goal{4, 0};
        const int expected = field.distance(goal);
        REQUIRE(expected > 0);

        NavCell at{0, 0};
        int taken = 0;
        while (!(at == goal) && taken <= expected)
        {
            const NavCell next = grid.flood(at).stepTowards(goal);
            REQUIRE_FALSE(next == at); // no stalling
            CHECK_FALSE(grid.isSolid(next));
            at = next;
            ++taken;
        }
        CHECK(at == goal);
        CHECK(taken == expected);
    }

    SUBCASE("the same flood always gives the same step")
    {
        // Determinism is the whole point: two --sim runs must match byte for
        // byte, so the tie-break between equal-length paths cannot vary.
        const NavCell first =
            grid.flood(NavCell{0, 0}).stepTowards(NavCell{4, 0});
        const NavCell again =
            grid.flood(NavCell{0, 0}).stepTowards(NavCell{4, 0});
        CHECK(first == again);
    }

    SUBCASE("a walled-off cell is unreachable, not distance zero")
    {
        NavGrid sealed;
        sealed.reset(5, 5, 16.0f);
        for (int row = 0; row < 5; ++row)
        {
            sealed.setSolid(2, row, true);
        }
        const NavField blocked = sealed.flood(NavCell{0, 0});
        CHECK_FALSE(blocked.reachable(NavCell{4, 0}));
        CHECK(blocked.distance(NavCell{4, 0}) == -1);

        // stepTowards must not invent a move towards somewhere unreachable.
        CHECK(blocked.stepTowards(NavCell{4, 0}) == NavCell{0, 0});
    }

    SUBCASE("a solid cell reaches nothing")
    {
        // Happens for one step while the player is being pushed out of a
        // wall; the policy has to detect it rather than stall.
        const NavField stuck = grid.flood(NavCell{2, 0});
        CHECK_FALSE(stuck.reachable(NavCell{0, 0}));
    }
}

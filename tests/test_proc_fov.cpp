#include <doctest/doctest.h>
#include <engine/spatial/visibility_map.hpp>
#include <game/proc/dungeon_generator.hpp>
#include <game/run/run_config.hpp>

TEST_CASE("dungeon generator is deterministic for a seed")
{
    RunConfig config;
    config.seed = 42;
    config.floorsPerRun = 3;
    config.objectiveFloor = 3;

    DungeonGenerator generator;
    const FloorBlueprint a = generator.generate(config, 1);
    const FloorBlueprint b = generator.generate(config, 1);

    CHECK(a.columns == b.columns);
    CHECK(a.rows == b.rows);
    CHECK(a.cells == b.cells);
    CHECK(a.markers.size() == b.markers.size());
}

TEST_CASE("dungeon generator places entrance and stairs on floor 1")
{
    RunConfig config;
    config.seed = 7;
    config.floorsPerRun = 3;
    config.objectiveFloor = 3;

    const FloorBlueprint floor = DungeonGenerator{}.generate(config, 1);
    bool hasEntrance = false;
    bool hasDown = false;
    bool hasObjective = false;
    for (const auto& marker : floor.markers)
    {
        if (marker.kind == FloorBlueprint::Marker::Kind::Entrance)
        {
            hasEntrance = true;
            CHECK(floor.at(marker.column, marker.row) ==
                  FloorBlueprint::Cell::Floor);
        }
        if (marker.kind == FloorBlueprint::Marker::Kind::StairsDown)
        {
            hasDown = true;
        }
        if (marker.kind == FloorBlueprint::Marker::Kind::Objective)
        {
            hasObjective = true;
        }
    }
    CHECK(hasEntrance);
    CHECK(hasDown);
    CHECK_FALSE(hasObjective);
}

TEST_CASE("visibility map remembers explored tiles")
{
    de::VisibilityMap map;
    map.reset(4, 4, 16.0f);
    map.setOpaque(0, 0, false);
    map.visible[0] = 1;
    map.explored[0] = 1;
    CHECK(map.appearanceAt(0.0f, 0.0f) ==
          de::VisibilityMap::Appearance::Visible);
    map.visible[0] = 0;
    CHECK(map.appearanceAt(0.0f, 0.0f) ==
          de::VisibilityMap::Appearance::Remembered);
    CHECK(map.appearanceAt(48.0f, 48.0f) ==
          de::VisibilityMap::Appearance::Hidden);
}

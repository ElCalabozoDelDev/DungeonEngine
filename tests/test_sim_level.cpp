#include <doctest/doctest.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <game/sim/nav_grid.hpp>
#include <string>
#include <vector>

using namespace de;

namespace
{
/// The real dungeon1.tmx, loaded the way the game loads it.
///
/// Everything the simulation policy claims about the level -- that the player
/// starts somewhere free, that every item can be approached -- depends on the
/// navigation grid agreeing with the loader. A synthetic grid cannot catch a
/// disagreement between them, so this one uses the actual asset.
///
/// No CameraBoundsComponent is emplaced here: that is InGameScene's job, so
/// this also covers NavGrid's fallback to sizing itself from the tiles.
struct RealLevel
{
    entt::registry registry;
    std::vector<entt::entity> entities;
    NavGrid grid;

    RealLevel()
    {
        // The loader looks up a TextureCache to load images into. With no
        // renderer it loads nothing, which is what lets this run headless.
        registry.ctx().emplace<TextureCache>(nullptr);

        TMXLoader loader(&entities);
        const auto loaded = loader.loadLevel(
            registry, std::string(DE_ASSETS_DIR) + "/Levels/dungeon1.tmx");
        REQUIRE(loaded.has_value());
        grid.build(registry);
    }

    std::vector<entt::entity> withType(const char* type) const
    {
        std::vector<entt::entity> found;
        for (auto entity : registry.view<const ObjectTypeComponent>())
        {
            if (registry.get<const ObjectTypeComponent>(entity).type == type)
            {
                found.push_back(entity);
            }
        }
        return found;
    }
};

} // namespace

TEST_CASE("the real level produces a navigable grid")
{
    RealLevel level;

    REQUIRE_FALSE(level.grid.empty());
    CHECK(level.grid.columns() == 26);
    CHECK(level.grid.rows() == 46);
    CHECK(level.grid.tileSize() == doctest::Approx(16.0f));

    const auto players = level.withType("Player");
    REQUIRE(players.size() == 1);

    const auto& transform =
        level.registry.get<TransformComponent>(players.front());
    const auto& dimension =
        level.registry.get<DimensionComponent>(players.front());
    const Vector2D<float> centre(
        transform.position.getX() + dimension.width * 0.5f,
        transform.position.getY() + dimension.height * 0.5f);
    const NavCell start = level.grid.cellAt(centre);

    SUBCASE("the player does not start inside a wall")
    {
        CHECK(start == NavCell{11, 45});
        CHECK_FALSE(level.grid.isSolid(start));
    }

    SUBCASE("most of the level is reachable from the start")
    {
        const NavField field = level.grid.flood(start);
        int reachable = 0;
        for (int row = 0; row < level.grid.rows(); ++row)
        {
            for (int column = 0; column < level.grid.columns(); ++column)
            {
                if (field.reachable(NavCell{column, row}))
                {
                    ++reachable;
                }
            }
        }
        // Measured by decoding the layers and flooding: 659 of 1196 cells.
        CHECK(reachable == 659);
    }

    SUBCASE("every item can be approached, including the one in the wall")
    {
        const NavField field = level.grid.flood(start);
        const auto items = level.withType("Item");
        REQUIRE(items.size() == 3);

        for (auto item : items)
        {
            const auto& itemTransform =
                level.registry.get<TransformComponent>(item);
            const auto& itemDimension =
                level.registry.get<DimensionComponent>(item);

            // Coin3 sits at (300,400): its own cell (19,25) is solid, and so
            // is every cell its box overlaps. The only way in is the cell
            // above, at (19,24) -- so an item is approachable if a cell it
            // touches *or* a neighbour of its centre is reachable, which is
            // the rule SeekItemsPolicy::approachCell implements.
            const NavCell centreCell = level.grid.cellAt(Vector2D<float>(
                itemTransform.position.getX() + itemDimension.width * 0.5f,
                itemTransform.position.getY() + itemDimension.height * 0.5f));

            bool approachable = false;
            const int columnOffset[] = {0, 1, -1, 0, 0};
            const int rowOffset[] = {0, 0, 0, 1, -1};
            for (int i = 0; i < 5; ++i)
            {
                approachable =
                    approachable ||
                    field.reachable(NavCell{centreCell.column + columnOffset[i],
                                            centreCell.row + rowOffset[i]});
            }
            CHECK(approachable);
        }
    }
}

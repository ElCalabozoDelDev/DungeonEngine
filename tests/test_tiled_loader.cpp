#include <doctest/doctest.h>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>

using namespace de;

namespace
{
std::filesystem::path fixture(const std::string& name)
{
    return std::filesystem::path(DE_TEST_FIXTURES) / name;
}

struct LoaderFixture
{
    entt::registry registry;
    std::vector<entt::entity> entities;

    LoaderFixture() { registry.ctx().emplace<TextureCache>(nullptr); }
};

} // namespace

TEST_CASE("a json-encoded level loads")
{
    LoaderFixture f;
    TiledLoader loader(&f.entities);

    auto result = loader.loadLevel(f.registry, fixture("minimal.tmj"));
    REQUIRE(result.has_value());

    CHECK(loader.getWidth() == 4);
    CHECK(loader.getHeight() == 4);
    CHECK(loader.getTileWidth() == 16);
    CHECK(loader.getTileHeight() == 16);

    SUBCASE("empty tiles are skipped")
    {
        CHECK(f.registry.view<TileComponent>().size() == 15);
    }

    SUBCASE("every tile knows its tileset")
    {
        auto tiles = f.registry.view<TileComponent>();
        for (auto entity : tiles)
        {
            const bool hasTileset =
                tiles.get<TileComponent>(entity).tileset != entt::null;
            CHECK(hasTileset);
        }
    }

    SUBCASE("tiles are placed on the grid")
    {
        auto tiles = f.registry.view<TileComponent, TransformComponent>();
        bool foundOrigin = false;
        for (auto entity : tiles)
        {
            const auto& p = tiles.get<TransformComponent>(entity).position;
            CHECK(p.getX() >= 0.0f);
            CHECK(p.getX() < 64.0f);
            CHECK(p.getY() >= 0.0f);
            CHECK(p.getY() < 64.0f);
            if (p.getX() == 0.0f && p.getY() == 0.0f)
            {
                foundOrigin = true;
            }
        }
        CHECK(foundOrigin);
    }

    SUBCASE("Bottom layer is tagged")
    {
        auto layers =
            f.registry.view<TileLayerComponent, BottomLayerComponent>();
        CHECK(std::distance(layers.begin(), layers.end()) == 1);
    }

    SUBCASE("objects keep their Tiled type")
    {
        auto objects = f.registry.view<ObjectTypeComponent>();
        REQUIRE(objects.size() == 2);
        bool foundPlayer = false;
        bool foundItem = false;
        for (auto entity : objects)
        {
            const auto& type = objects.get<ObjectTypeComponent>(entity).type;
            if (type == "Player")
            {
                foundPlayer = true;
            }
            if (type == "Item")
            {
                foundItem = true;
            }
        }
        CHECK(foundPlayer);
        CHECK(foundItem);
    }

    SUBCASE("tileset is recorded")
    {
        CHECK(f.registry.view<TileSetComponent>().size() == 1);
    }
}

TEST_CASE("a missing file is reported")
{
    LoaderFixture f;
    TiledLoader loader(&f.entities);
    auto result = loader.loadLevel(f.registry, fixture("no_such.tmj"));
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("a layer with the wrong tile count is reported")
{
    LoaderFixture f;
    TiledLoader loader(&f.entities);
    auto result = loader.loadLevel(f.registry, fixture("no_data.tmj"));
    REQUIRE_FALSE(result.has_value());
}

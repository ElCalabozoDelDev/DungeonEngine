#include <doctest/doctest.h>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <string>
#include <vector>

using namespace de;

namespace
{
/// Where the fixture .tmx files live, injected by CMake so the test does not
/// care what directory it is run from.
std::filesystem::path fixture(const std::string& name)
{
    return std::filesystem::path(DE_TEST_FIXTURES) / name;
}

/// A registry with just enough context for the loader: it looks up a
/// TextureCache to load images into. The cache has no renderer, so image
/// loading is a no-op -- which is what lets these run without a window.
struct LoaderFixture
{
    entt::registry registry;
    std::vector<entt::entity> entities;

    LoaderFixture() { registry.ctx().emplace<TextureCache>(nullptr); }
};

} // namespace

TEST_CASE("a csv-encoded level loads")
{
    LoaderFixture f;
    TMXLoader loader(&f.entities);

    auto result = loader.loadLevel(f.registry, fixture("minimal.tmx"));
    REQUIRE(result.has_value());

    CHECK(loader.getWidth() == 4);
    CHECK(loader.getHeight() == 4);
    CHECK(loader.getTileWidth() == 16);
    CHECK(loader.getTileHeight() == 16);

    SUBCASE("empty tiles are skipped")
    {
        // 4x4 with one gid of 0 in it.
        CHECK(f.registry.view<TileComponent>().size() == 15);
    }

    SUBCASE("every tile knows its tileset")
    {
        // Resolved once at load time; the renderer used to search for it on
        // every visible tile of every frame, and returned an uninitialised
        // entity when it found nothing.
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
            CHECK(p.getX() < 64.0f); // 4 tiles * 16px
            if (p.getX() == 0.0f && p.getY() == 0.0f)
            {
                foundOrigin = true;
            }
        }
        CHECK(foundOrigin);
    }

    SUBCASE("the layer is tagged from its name")
    {
        CHECK(f.registry.view<BottomLayerComponent>().size() == 1);
    }

    SUBCASE("the tileset is read")
    {
        auto tilesets = f.registry.view<TileSetComponent>();
        REQUIRE(tilesets.size() == 1);
        const auto& tileset = tilesets.get<TileSetComponent>(*tilesets.begin());
        CHECK(tileset.firstGridID == 1);
        CHECK(tileset.numColumns == 2); // 32px image / 16px tiles
    }
}

TEST_CASE("objects carry their Tiled type, and nothing more")
{
    // The loader must not name a gameplay component: it records the `type`
    // string and the game decides what "Player" means.
    LoaderFixture f;
    TMXLoader loader(&f.entities);
    REQUIRE(loader.loadLevel(f.registry, fixture("minimal.tmx")).has_value());

    auto objects = f.registry.view<ObjectTypeComponent>();
    REQUIRE(objects.size() == 2);

    std::vector<std::string> types;
    for (auto entity : objects)
    {
        types.push_back(objects.get<ObjectTypeComponent>(entity).type);
    }
    CHECK(std::find(types.begin(), types.end(), "Player") != types.end());
    CHECK(std::find(types.begin(), types.end(), "Item") != types.end());
}

TEST_CASE("a level that is not csv-encoded is reported")
{
    LoaderFixture f;
    TMXLoader loader(&f.entities);

    auto result =
        loader.loadLevel(f.registry, fixture("unsupported_encoding.tmx"));
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find("csv") != std::string::npos);
}

TEST_CASE("a layer with no data element is reported")
{
    // The data node pointer was declared uninitialised and dereferenced.
    LoaderFixture f;
    TMXLoader loader(&f.entities);

    auto result = loader.loadLevel(f.registry, fixture("no_data.tmx"));
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find("data") != std::string::npos);
}

TEST_CASE("a missing level file is reported")
{
    LoaderFixture f;
    TMXLoader loader(&f.entities);

    auto result = loader.loadLevel(f.registry, fixture("no_such_level.tmx"));
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find("could not read") != std::string::npos);
}

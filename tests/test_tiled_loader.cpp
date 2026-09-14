#include <doctest/doctest.h>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <fstream>
#include <initializer_list>
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

    LoaderFixture() { registry.ctx().emplace<TextureCache>(nullptr); }
};

/// Writes a .tmj next to the fixtures' tileset, so relative paths resolve.
class TempLevel
{
public:
    TempLevel(const std::string& name, const std::string& json)
        : m_path(std::filesystem::temp_directory_path() / name)
    {
        std::ofstream(m_path) << json;
    }
    ~TempLevel()
    {
        std::error_code ec;
        std::filesystem::remove(m_path, ec);
    }
    const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

/// A 1x1 map with the given tile layers, each holding tile id 0 (empty).
std::string mapWithLayers(std::initializer_list<const char*> names)
{
    std::string layers;
    for (const char* name : names)
    {
        if (!layers.empty())
        {
            layers += ",";
        }
        layers += std::string(R"({"type":"tilelayer","name":")") + name +
                  R"(","data":[0]})";
    }
    return R"({"width":1,"height":1,"tilewidth":16,"tileheight":16,"layers":[)" +
           layers + "]}";
}

} // namespace

TEST_CASE("a json-encoded level loads")
{
    LoaderFixture f;
    auto result = TiledLoader().load(f.registry, fixture("minimal.tmj"));
    REQUIRE(result.has_value());

    CHECK(result->width == 4);
    CHECK(result->height == 4);
    CHECK(result->tileWidth == 16);
    CHECK(result->tileHeight == 16);

    SUBCASE("every entity it created is reported")
    {
        const auto created = static_cast<std::size_t>(
            std::distance(f.registry.storage<entt::entity>().each().begin(),
                          f.registry.storage<entt::entity>().each().end()));
        CHECK(result->entities.size() == created);
    }

    SUBCASE("objects are not given a velocity")
    {
        // Movement is the game's to add; the loader used to give every
        // object a VelocityComponent whether it moved or not.
        CHECK(f.registry.view<VelocityComponent>().empty());
    }

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
            CHECK(p.x >= 0.0f);
            CHECK(p.x < 64.0f);
            CHECK(p.y >= 0.0f);
            CHECK(p.y < 64.0f);
            if (p.x == 0.0f && p.y == 0.0f)
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
    auto result = TiledLoader().load(f.registry, fixture("no_such.tmj"));
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("a layer with the wrong tile count is reported")
{
    LoaderFixture f;
    auto result = TiledLoader().load(f.registry, fixture("no_data.tmj"));
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("a load that fails part-way leaves no entities behind")
{
    // The first layer loads, the second has the wrong tile count. The layer
    // entity the first one created used to be handed to the scene through an
    // out-parameter; with nothing returned on failure it would leak instead.
    LoaderFixture f;
    TempLevel level("de_partial.tmj",
                    R"({"width":1,"height":1,"tilewidth":16,"tileheight":16,
                        "layers":[{"type":"tilelayer","name":"Bottom","data":[0]},
                                  {"type":"tilelayer","name":"Overlay","data":[0,0]}]})");

    auto result = TiledLoader().load(f.registry, level.path());
    REQUIRE_FALSE(result.has_value());
    CHECK(f.registry.view<TileLayerComponent>().empty());
    CHECK(f.registry.storage<entt::entity>().free_list() == 0);
}

TEST_CASE("tile layers are placed by name")
{
    LoaderFixture f;

    SUBCASE("Bottom and Overlay by their default names")
    {
        TempLevel level("de_named.tmj",
                        mapWithLayers({"Bottom", "Decor", "Overlay"}));
        REQUIRE(TiledLoader().load(f.registry, level.path()).has_value());
        CHECK(f.registry.view<BottomLayerComponent>().size() == 1);
        CHECK(f.registry.view<OverlayLayerComponent>().size() == 1);
    }

    SUBCASE("a single tile layer is drawn under the objects, whatever its name")
    {
        TempLevel level("de_single.tmj", mapWithLayers({"Tile Layer 1"}));
        REQUIRE(TiledLoader().load(f.registry, level.path()).has_value());
        CHECK(f.registry.view<BottomLayerComponent>().size() == 1);
    }

    SUBCASE("with names the game chooses")
    {
        TiledLayerNames names;
        names.bottom = {"Floor"};
        names.overlay = {"Roof"};
        TempLevel level("de_custom.tmj",
                        mapWithLayers({"Floor", "Bottom", "Roof"}));
        REQUIRE(TiledLoader(names).load(f.registry, level.path()).has_value());
        CHECK(f.registry.view<BottomLayerComponent>().size() == 1);
        CHECK(f.registry.view<OverlayLayerComponent>().size() == 1);
    }
}

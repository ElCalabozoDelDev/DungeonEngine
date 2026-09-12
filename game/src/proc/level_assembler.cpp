#include <engine/components/animation_component.hpp>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/collision_component.hpp>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/level_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <game/proc/level_assembler.hpp>

using namespace de;

namespace
{
constexpr int FloorGid = 90;
constexpr int WallGid = 15;
constexpr int TileColumns = 10;
constexpr int FirstGid = 1;

entt::entity makeObject(entt::registry& registry,
                        std::vector<entt::entity>* entities,
                        const std::string& type, float x, float y,
                        const std::string& texId, int spriteRow, int spriteCol)
{
    auto entity = registry.create();
    registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
    registry.emplace<DimensionComponent>(entity, 16.0f, 16.0f);
    registry.emplace<TextureComponent>(entity, texId);
    registry.emplace<SpriteComponent>(entity, spriteRow, spriteCol, 0);
    registry.emplace<VelocityComponent>(entity, Vector2D<float>(0, 0));
    AnimationComponent animation;
    animation.currentFrame = 0;
    animation.totalFrames = 1;
    animation.animationTime = 0.15f;
    registry.emplace<AnimationComponent>(entity, animation);
    registry.emplace<ObjectTypeComponent>(entity, type);
    entities->push_back(entity);
    return entity;
}
} // namespace

std::expected<void, std::string>
LevelAssembler::assemble(entt::registry& registry, const FloorBlueprint& floor,
                         const AssetPaths& assets)
{
    if (floor.columns <= 0 || floor.rows <= 0)
    {
        return std::unexpected("floor blueprint has empty dimensions");
    }

    auto& textures = registry.ctx().get<TextureCache>();
    if (!textures.load("tiles_dungeon",
                       assets.resolve("Tiles/1-Dungeon.png").string()) ||
        !textures.load(
            "player",
            assets.resolve("Player/1-Heroes-Animated.png").string()) ||
        !textures.load(
            "monster1",
            assets.resolve("Enemies/1-Monsters-Animated.png").string()) ||
        !textures.load("items", assets.resolve("Items/1-Items.png").string()))
    {
        return std::unexpected("failed to load procedural level textures");
    }

    auto tileset = registry.create();
    {
        TileSetComponent tilesetComp;
        tilesetComp.firstGridID = FirstGid;
        tilesetComp.spacing = 0;
        tilesetComp.margin = 0;
        tilesetComp.numColumns = TileColumns;
        tilesetComp.tileCount = 100;
        registry.emplace<TileSetComponent>(tileset, tilesetComp);
    }
    registry.emplace<TextureComponent>(tileset, "tiles_dungeon");
    registry.emplace<DimensionComponent>(tileset, floor.tileSize,
                                         floor.tileSize);
    m_entities->push_back(tileset);

    auto bottomLayer = registry.create();
    registry.emplace<TileLayerComponent>(bottomLayer);
    registry.emplace<BottomLayerComponent>(bottomLayer);
    m_entities->push_back(bottomLayer);

    auto collisionLayer = registry.create();
    registry.emplace<TileLayerComponent>(collisionLayer);
    registry.emplace<CollisionLayerComponent>(collisionLayer);
    registry.emplace<CollisionComponent>(collisionLayer);
    m_entities->push_back(collisionLayer);

    auto& bottomTiles = registry.get<TileLayerComponent>(bottomLayer);
    auto& collisionTiles = registry.get<TileLayerComponent>(collisionLayer);

    for (int row = 0; row < floor.rows; ++row)
    {
        for (int col = 0; col < floor.columns; ++col)
        {
            const auto cell = floor.at(col, row);
            if (cell == FloorBlueprint::Cell::Void)
            {
                continue;
            }

            const float x = static_cast<float>(col) * floor.tileSize;
            const float y = static_cast<float>(row) * floor.tileSize;

            if (cell == FloorBlueprint::Cell::Floor)
            {
                auto tile = registry.create();
                registry.emplace<TileComponent>(tile, FloorGid, tileset);
                registry.emplace<TransformComponent>(tile,
                                                     Vector2D<float>(x, y));
                registry.emplace<DimensionComponent>(tile, floor.tileSize,
                                                     floor.tileSize);
                bottomTiles.tileEntities.push_back(tile);
                m_entities->push_back(tile);
            }
            else if (cell == FloorBlueprint::Cell::Wall)
            {
                auto tile = registry.create();
                registry.emplace<TileComponent>(tile, WallGid, tileset);
                registry.emplace<TransformComponent>(tile,
                                                     Vector2D<float>(x, y));
                registry.emplace<DimensionComponent>(tile, floor.tileSize,
                                                     floor.tileSize);
                collisionTiles.tileEntities.push_back(tile);
                m_entities->push_back(tile);
            }
        }
    }

    for (const auto& marker : floor.markers)
    {
        const float x = static_cast<float>(marker.column) * floor.tileSize;
        const float y = static_cast<float>(marker.row) * floor.tileSize;
        switch (marker.kind)
        {
            case FloorBlueprint::Marker::Kind::Entrance:
                makeObject(registry, m_entities, "Player", x, y, "player", 0,
                           0);
                break;
            case FloorBlueprint::Marker::Kind::StairsDown:
                makeObject(registry, m_entities, "StairsDown", x, y, "items", 0,
                           3);
                break;
            case FloorBlueprint::Marker::Kind::StairsUp:
                makeObject(registry, m_entities, "StairsUp", x, y, "items", 0,
                           4);
                break;
            case FloorBlueprint::Marker::Kind::Enemy:
                makeObject(registry, m_entities, "Enemy", x, y, "monster1", 0,
                           0);
                break;
            case FloorBlueprint::Marker::Kind::ScoreItem:
                makeObject(registry, m_entities, "Item", x, y, "items", 0, 0);
                break;
            case FloorBlueprint::Marker::Kind::Objective:
                makeObject(registry, m_entities, "Objective", x, y, "items", 0,
                           2);
                break;
        }
    }

    auto levelEntity = registry.create();
    auto& level = registry.emplace<LevelComponent>(levelEntity);
    level.tilesets = {tileset};
    level.layers = {bottomLayer, collisionLayer};
    m_entities->push_back(levelEntity);

    m_mapPixelWidth =
        static_cast<int>(static_cast<float>(floor.columns) * floor.tileSize);
    m_mapPixelHeight =
        static_cast<int>(static_cast<float>(floor.rows) * floor.tileSize);
    return {};
}

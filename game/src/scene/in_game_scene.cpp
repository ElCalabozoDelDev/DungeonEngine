#include <SDL_render.h>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/follow_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/constants.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/render_collision.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/render_overlay.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <engine/spatial/quadtree_manager.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/components/player_component.hpp>
#include <game/debug/player_editor.hpp>
#include <game/scene/in_game_scene.hpp>
#include <iostream>

using namespace de;

InGameScene::InGameScene()
{
    // Nothing to set up until onEnter: the registry does not exist yet.
}

void InGameScene::onEnter(entt::registry& registry)
{
    const auto& config = registry.ctx().get<Config>();

    // Load the level
    TMXLoader tmxLoader(&m_entities);
    tmxLoader.loadLevel(registry, config.levels.at("level1").c_str());

    // Turn the Tiled object types into this game's components
    tagObjectsByType(registry);

    // Set up the camera
    int mapWidth = tmxLoader.getWidth() * tmxLoader.getTileWidth();
    int mapHeight = tmxLoader.getHeight() * tmxLoader.getTileHeight();
    initializeCamera(registry, mapWidth, mapHeight, config);

    // Spatial indices and render passes
    initializeQuadtrees(registry, static_cast<float>(mapWidth),
                        static_cast<float>(mapHeight));
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
    initializeRenderers(registry);
    initializeDebug(registry, config.debug);
}

void InGameScene::onUpdate(entt::registry& registry) {}

void InGameScene::onExit(entt::registry& registry)
{
    // Destroy every entity this scene created
    for (auto entity : m_entities)
    {
        registry.destroy(entity);
    }
    m_entities.clear();
}

// The TMX loader records each object's Tiled `type` without interpreting it;
// mapping those strings onto gameplay components is this game's decision.
void InGameScene::tagObjectsByType(entt::registry& registry)
{
    auto view = registry.view<ObjectTypeComponent>();
    for (auto entity : view)
    {
        const auto& objectType = view.get<ObjectTypeComponent>(entity);
        if (objectType.type == "Player")
        {
            registry.emplace<PlayerComponent>(entity);
        }
    }
}

void InGameScene::initializeQuadtrees(entt::registry& registry, float mapWidth,
                                      float mapHeight)
{
    std::function<Box<float>(const entt::entity&)> getBox =
        [&registry](const entt::entity& entity)
    {
        auto& transform = registry.get<TransformComponent>(entity);
        auto& dimension = registry.get<DimensionComponent>(entity);
        return Box<float>(transform.position.getX(), transform.position.getY(),
                          static_cast<float>(dimension.width),
                          static_cast<float>(dimension.height));
    };

    QuadtreeManager::Instance()->createQuadtree(
        getBox, LayerType::BOTTOM, Box<float>(0.0f, 0.0f, mapWidth, mapHeight));
    QuadtreeManager::Instance()->createQuadtree(
        getBox, LayerType::OVERLAY,
        Box<float>(0.0f, 0.0f, mapWidth, mapHeight));
    QuadtreeManager::Instance()->createQuadtree(
        getBox, LayerType::COLLISION,
        Box<float>(0.0f, 0.0f, mapWidth, mapHeight));
    QuadtreeManager::Instance()->createQuadtree(
        getBox, LayerType::OBJECT, Box<float>(0.0f, 0.0f, mapWidth, mapHeight));
}

void InGameScene::populateTileQuadtree(entt::registry& registry)
{
    auto bottomView = registry.view<TileLayerComponent, BottomLayerComponent>();
    for (auto entity : bottomView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            QuadtreeManager::Instance()->insertIntoQuadtree(LayerType::BOTTOM,
                                                            tile);
        }
    }

    auto overlayView =
        registry.view<TileLayerComponent, OverlayLayerComponent>();
    for (auto entity : overlayView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            QuadtreeManager::Instance()->insertIntoQuadtree(LayerType::OVERLAY,
                                                            tile);
        }
    }

    auto collisionView =
        registry.view<TileLayerComponent, CollisionLayerComponent>();
    for (auto entity : collisionView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            QuadtreeManager::Instance()->insertIntoQuadtree(
                LayerType::COLLISION, tile);
        }
    }
}

void InGameScene::populateSpriteQuadtree(entt::registry& registry)
{
    auto view = registry.view<SpriteComponent>();
    for (auto entity : view)
    {
        QuadtreeManager::Instance()->insertIntoQuadtree(LayerType::OBJECT,
                                                        entity);
    }
}

void InGameScene::initializeCamera(entt::registry& registry, int mapWidth,
                                   int mapHeight, const Config& config)
{
    auto cameraEntity = registry.create();
    registry.emplace<DimensionComponent>(cameraEntity,
                                         static_cast<int>(config.cameraWidth),
                                         static_cast<int>(config.cameraHeight));

    auto& camera = registry.emplace<CameraComponent>(cameraEntity);
    camera.viewportOffsetX = config.viewportOffsetX;
    camera.viewportOffsetY = config.viewportOffsetY;
    camera.zoomLevel = config.zoomLevel;
    registry.emplace<CameraBoundsComponent>(cameraEntity, mapWidth, mapHeight);

    auto view = registry.view<PlayerComponent>();
    if (view.empty())
    {
        std::cerr << "No player found in the scene" << std::endl;
        return;
    }

    auto player = *view.begin();
    auto& playerTransform = registry.get<TransformComponent>(player).position;
    registry.emplace<TransformComponent>(cameraEntity, playerTransform);
    registry.emplace<FollowComponent>(cameraEntity, player);
    m_entities.push_back(cameraEntity);
}

void InGameScene::initializeRenderers(entt::registry& registry)
{
    auto bottomEntity = registry.create();
    auto overlayEntity = registry.create();
    auto collisionEntity = registry.create();
    auto objectEntity = registry.create();

    registry.emplace<std::shared_ptr<Render>>(
        collisionEntity, std::make_shared<RenderCollision>());
    registry.emplace<std::shared_ptr<Render>>(objectEntity,
                                              std::make_shared<RenderObject>());
    registry.emplace<std::shared_ptr<Render>>(
        overlayEntity, std::make_shared<RenderOverlay>());
    registry.emplace<std::shared_ptr<Render>>(bottomEntity,
                                              std::make_shared<RenderBottom>());
}

void InGameScene::initializeDebug(entt::registry& registry, bool open)
{
    if (!open)
        return;

    auto& debugSystem = *registry.ctx().get<std::shared_ptr<DebugSystem>>();
    debugSystem.setOpen(open);
    debugSystem.register_component<PlayerComponent>("Player");
    debugSystem.register_component<CameraComponent>("Camera");
}

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
#include <engine/core/asset_paths.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/render_collision.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/render_overlay.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/debug_system.hpp>
#include <engine/widgets/widget.hpp>
#include <game/components/player_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/debug/player_editor.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/widgets/counter_widget.hpp>
#include <iostream>

using namespace de;

InGameScene::InGameScene()
{
    // Nothing to set up until onEnter: the registry does not exist yet.
}

// onEnter runs inside the setup hook, so a failure here is a startup failure:
// GameLoop reports it and exits rather than leaving an empty black window up.
void InGameScene::fail(entt::registry& registry, const std::string& reason)
{
    registry.ctx().emplace<StartupError>(StartupError{"level: " + reason});
}

void InGameScene::onEnter(entt::registry& registry)
{
    const auto& config = registry.ctx().get<Config>();

    // Load the level
    const auto& assets = registry.ctx().get<AssetPaths>();
    auto level = config.levels.find("level1");
    if (level == config.levels.end())
    {
        fail(registry, "game.xml declares no level named 'level1'");
        return;
    }

    TMXLoader tmxLoader(&m_entities);
    if (auto loaded =
            tmxLoader.loadLevel(registry, assets.resolve(level->second));
        !loaded)
    {
        fail(registry, loaded.error());
        return;
    }

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
    // Drop the spatial trees first: they hold entity handles, and a query
    // after the entities are gone would hand back dangling ones.
    registry.ctx().get<SpatialIndex>().clear();

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
            registry.emplace<SpeedComponent>(entity);
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

    const Box<float> bounds(0.0f, 0.0f, mapWidth, mapHeight);
    auto& spatial = registry.ctx().get<SpatialIndex>();
    spatial.create(Layer::Bottom, getBox, bounds);
    spatial.create(Layer::Overlay, getBox, bounds);
    spatial.create(Layer::Collision, getBox, bounds);
    spatial.create(Layer::Object, getBox, bounds);
}

void InGameScene::populateTileQuadtree(entt::registry& registry)
{
    auto& spatial = registry.ctx().get<SpatialIndex>();

    auto bottomView = registry.view<TileLayerComponent, BottomLayerComponent>();
    for (auto entity : bottomView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            spatial.insert(Layer::Bottom, tile);
        }
    }

    auto overlayView =
        registry.view<TileLayerComponent, OverlayLayerComponent>();
    for (auto entity : overlayView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            spatial.insert(Layer::Overlay, tile);
        }
    }

    auto collisionView =
        registry.view<TileLayerComponent, CollisionLayerComponent>();
    for (auto entity : collisionView)
    {
        auto& tileLayer = registry.get<TileLayerComponent>(entity);
        for (auto tile : tileLayer.tileEntities)
        {
            spatial.insert(Layer::Collision, tile);
        }
    }
}

void InGameScene::populateSpriteQuadtree(entt::registry& registry)
{
    auto& spatial = registry.ctx().get<SpatialIndex>();
    auto view = registry.view<SpriteComponent>();
    for (auto entity : view)
    {
        spatial.insert(Layer::Object, entity);
    }
}

void InGameScene::initializeCamera(entt::registry& registry, int mapWidth,
                                   int mapHeight, const Config& config)
{
    auto cameraEntity = registry.create();
    registry.emplace<DimensionComponent>(cameraEntity, config.cameraWidth,
                                         config.cameraHeight);

    auto& camera = registry.emplace<CameraComponent>(cameraEntity);
    camera.zoomLevel = config.zoomLevel;
    registry.emplace<CameraBoundsComponent>(cameraEntity, mapWidth, mapHeight);

    auto view = registry.view<PlayerComponent>();
    if (view.empty())
    {
        fail(registry,
             "the level contains no object with type=\"Player\"; there is "
             "nothing for the camera to follow");
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
    // Order is declared, not implied by creation order. They are created here
    // deliberately out of draw order to make that obvious.
    const auto addPass = [&](std::unique_ptr<Render> pass, int order)
    {
        auto entity = registry.create();
        registry.emplace<RenderPass>(entity, std::move(pass), order);
        m_entities.push_back(entity);
    };

    addPass(std::make_unique<RenderOverlay>(), render_order::Overlay);
    addPass(std::make_unique<RenderObject>(), render_order::Object);
    addPass(std::make_unique<RenderBottom>(), render_order::Bottom);
    addPass(std::make_unique<RenderCollision>(), render_order::Collision);
}

void InGameScene::initializeDebug(entt::registry& registry, bool open)
{
    if (!open)
        return;

    auto& debugSystem = *registry.ctx().get<std::shared_ptr<DebugSystem>>();
    debugSystem.setOpen(open);
    debugSystem.register_component<PlayerComponent>("Player");
    debugSystem.register_component<CameraComponent>("Camera");

    // The hook-based widget layer, actually in use: WidgetPlugin used to be
    // an empty shell and CounterWidget was never instantiated.
    auto widgetEntity = registry.create();
    registry.emplace<Widget>(widgetEntity, std::make_unique<CounterWidget>());
    m_entities.push_back(widgetEntity);
}

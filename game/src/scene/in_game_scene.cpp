#include "scene/in_game_scene.hpp"
#include "SDL_render.h"
#include "components/bottom_layer_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/collision_layer_component.hpp"
#include "components/follow_component.hpp"
#include "components/overlay_layer_component.hpp"
#include "components/player_component.hpp"
#include "components/sprite_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/transform_component.hpp"
#include "core/constants.hpp"
#include "core/quadtree_manager.hpp"
#include "graphics/render_bottom.hpp"
#include "graphics/render_collision.hpp"
#include "graphics/render_object.hpp"
#include "graphics/render_overlay.hpp"
#include "loaders/tmx_loader.hpp"
#include "systems/debug_system.hpp"


InGameScene::InGameScene()
{
    // Inicialización de la escena del juego
}

// Método que se ejecuta cuando se entra a la escena
void InGameScene::onEnter(entt::registry& registry)
{
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    auto config = registry.ctx().get<Config>();

    // Cargar el nivel
    TMXLoader tmxLoader(&m_entities);
    tmxLoader.loadLevel(registry, config.levels["level1"].c_str());

    // Inicializar la cámara
    int mapWidth = tmxLoader.getWidth() * tmxLoader.getTileWidth();
    int mapHeight = tmxLoader.getHeight() * tmxLoader.getTileHeight();
    initializeCamera(registry, mapWidth, mapHeight, config);

    // Inicializar quadtrees y otros componentes
    initializeQuadtrees(registry, mapWidth, mapHeight);
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
    initializeRenderers(registry);
    initializeDebug(registry, config.debug);
}

// Método que se llama en cada frame para actualizar la escena
void InGameScene::onUpdate(entt::registry& registry) {}

// Método que se ejecuta cuando se sale de la escena
void InGameScene::onExit(entt::registry& registry)
{
    // Destruir todas las entidades creadas en esta escena
    for (auto entity : m_entities)
    {
        registry.destroy(entity);
    }
    m_entities.clear();
}

// Inicializa los Quadtrees para la escena
void InGameScene::initializeQuadtrees(entt::registry& registry, float mapWidth,
                                      float mapHeight)
{
    std::function<Box<float>(const entt::entity&)> getBox =
        [&registry](const entt::entity& entity)
    {
        auto& transform = registry.get<TransformComponent>(entity);
        auto& dimension = registry.get<DimensionComponent>(entity);
        return quadtree::Box<float>(transform.position.getX(),
                                    transform.position.getY(), dimension.width,
                                    dimension.height);
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

// Población del Quadtree con entidades de tile
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

// Población del Quadtree con entidades de sprite
void InGameScene::populateSpriteQuadtree(entt::registry& registry)
{
    auto view = registry.view<SpriteComponent>();
    for (auto entity : view)
    {
        QuadtreeManager::Instance()->insertIntoQuadtree(LayerType::OBJECT,
                                                        entity);
    }
}

// Inicializa la cámara de la escena
void InGameScene::initializeCamera(entt::registry& registry, int mapWidth,
                                   int mapHeight, Config config)
{
    auto cameraEntity = registry.create();
    registry.emplace<DimensionComponent>(cameraEntity, config.cameraWidth,
                                         config.cameraHeight);

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

// Inicializa los renderizadores de la escena
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

// Inicializa el sistema de depuración
void InGameScene::initializeDebug(entt::registry& registry, bool open)
{
    if (!open)
        return;

    auto& debugSystem = *registry.ctx().get<std::shared_ptr<DebugSystem>>();
    debugSystem.setOpen(open);
    debugSystem.register_component<PlayerComponent>("Player");
    debugSystem.register_component<CameraComponent>("Camera");
}

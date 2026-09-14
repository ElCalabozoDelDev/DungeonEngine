#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/asset_paths.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/render_overlay.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <engine/spatial/level_spatial_index.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/debug_system.hpp>
#include <engine/widgets/widget.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/play_state.hpp>
#include <game/prefabs.hpp>
#include <game/rng.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/hud_widget.hpp>
#include <memory>

using namespace de;

InGameScene::InGameScene() = default;

void InGameScene::fail(entt::registry& registry, const std::string& reason)
{
    registry.ctx().emplace<StartupError>(StartupError{"level: " + reason});
}

void InGameScene::onEnter(entt::registry& registry)
{
    const auto& config = registry.ctx().get<Config>();

    // A new run: reset the state GamePlugin installed, rather than creating it.
    registry.ctx().get<GameState>() = GameState{};
    setPlayState(registry, PlayState::Playing);

    if (!loadLevel(registry, config))
    {
        return;
    }

    startMusic(registry);
    spawnPlayer(registry);
    spawnBat(registry);
    initializeCamera(registry, config);

    const auto& state = registry.ctx().get<GameState>();
    buildLevelSpatialIndex(
        registry, static_cast<float>(state.mapColumns * state.tileWidth),
        static_cast<float>(state.mapRows * state.tileHeight));

    initializeRenderers(registry);
    initializeHud(registry);
    initializeDebug(registry, config.debug);
}

void InGameScene::onUpdate(entt::registry& /*registry*/) {}

void InGameScene::onExit(entt::registry& registry)
{
    registry.ctx().get<SpatialIndex>().clear();
    resetRunPresentation(registry);

    for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
    {
        auto& snake = registry.get<SnakeComponent>(entity);
        for (auto segment : snake.segmentEntities)
        {
            if (registry.valid(segment))
            {
                registry.destroy(segment);
            }
        }
        snake.segmentEntities.clear();
    }

    for (auto entity : m_entities)
    {
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }
    m_entities.clear();
}

bool InGameScene::loadLevel(entt::registry& registry, const Config& config)
{
    auto level = config.levels.find("arena");
    if (level == config.levels.end())
    {
        fail(registry, "game.json declares no level named 'arena'");
        return false;
    }

    const auto& assets = registry.ctx().get<AssetPaths>();
    TiledLoader loader(&m_entities);
    if (auto loaded = loader.loadLevel(registry, assets.resolve(level->second));
        !loaded)
    {
        fail(registry, loaded.error());
        return false;
    }

    auto& state = registry.ctx().get<GameState>();
    state.tileWidth = loader.getTileWidth();
    state.tileHeight = loader.getTileHeight();
    state.mapColumns = loader.getWidth();
    state.mapRows = loader.getHeight();
    // One-tile inset playable room, matching the MonoGame tutorial.
    state.roomBounds =
        Box<float>(static_cast<float>(state.tileWidth),
                   static_cast<float>(state.tileHeight),
                   static_cast<float>((state.mapColumns - 2) * state.tileWidth),
                   static_cast<float>((state.mapRows - 2) * state.tileHeight));
    return true;
}

void InGameScene::startMusic(entt::registry& registry)
{
    // Theme is started on Title and must keep playing; only start it if
    // something entered InGame without going through Title (e.g. --level).
    if (auto* audio = registry.ctx().find<AudioManager>();
        audio != nullptr && !audio->isMusicPlaying())
    {
        audio->playMusic("theme", true);
    }
}

void InGameScene::spawnPlayer(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    const float tile = static_cast<float>(state.tileWidth);

    // Tiled's "Player" objects become the snake head, snapped to the centre
    // of the tile they sit in so stride steps stay flush with the wall cells.
    auto view = registry.view<ObjectTypeComponent, TransformComponent>();
    for (auto entity : view)
    {
        if (view.get<ObjectTypeComponent>(entity).type != "Player")
        {
            continue;
        }
        const Vector2D<float> pos =
            view.get<TransformComponent>(entity).position;
        const int col = static_cast<int>(std::floor(pos.getX() / tile));
        const int row = static_cast<int>(std::floor(pos.getY() / tile));
        const Vector2D<float> center(
            static_cast<float>(col) * tile + tile * 0.5f,
            static_cast<float>(row) * tile + tile * 0.5f);
        game::prefab::makeSnakeHead(registry, entity, center, tile);
    }

    // If the map had no Player object, spawn at the room centre.
    if (registry.view<PlayerComponent>().empty())
    {
        const Vector2D<float> center(
            state.roomBounds.getLeft() + state.roomBounds.getWidth() * 0.5f,
            state.roomBounds.getTop() + state.roomBounds.getHeight() * 0.5f);
        auto entity = registry.create();
        game::prefab::makeSnakeHead(registry, entity, center, tile);
        m_entities.push_back(entity);
    }
}

void InGameScene::spawnBat(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    auto& rng = registry.ctx().get<GameRng>().engine;
    m_entities.push_back(
        game::prefab::makeBat(registry, state.roomBounds, rng));
}

void InGameScene::initializeCamera(entt::registry& registry,
                                   const Config& config)
{
    // Fixed camera covering the full logical view; no FollowComponent.
    auto cameraEntity = registry.create();
    registry.emplace<DimensionComponent>(cameraEntity, config.cameraWidth,
                                         config.cameraHeight);
    auto& camera = registry.emplace<CameraComponent>(cameraEntity);
    camera.zoomLevel = config.zoomLevel;
    registry.emplace<TransformComponent>(
        cameraEntity,
        Vector2D<float>(config.cameraWidth * 0.5f, config.cameraHeight * 0.5f));
    m_entities.push_back(cameraEntity);
}

void InGameScene::initializeRenderers(entt::registry& registry)
{
    const auto addPass = [&](std::unique_ptr<Render> pass, int order)
    {
        auto entity = registry.create();
        registry.emplace<RenderPass>(entity, std::move(pass), order);
        m_entities.push_back(entity);
    };

    addPass(std::make_unique<RenderOverlay>(), render_order::Overlay);
    addPass(std::make_unique<RenderObject>(), render_order::Object);
    addPass(std::make_unique<RenderBottom>(), render_order::Bottom);
}

void InGameScene::initializeHud(entt::registry& registry)
{
    auto hudEntity = registry.create();
    registry.emplace<Widget>(hudEntity, std::make_unique<HudWidget>());
    m_entities.push_back(hudEntity);
}

void InGameScene::initializeDebug(entt::registry& registry, bool open)
{
    if (!open)
    {
        return;
    }

    auto& debugSystem = registry.ctx().get<DebugSystem>();
    debugSystem.setOpen(open);
    debugSystem.register_component<PlayerComponent>("Player");
    debugSystem.register_component<CameraComponent>("Camera");
    debugSystem.register_component<SnakeComponent>("Snake");
    debugSystem.register_component<BatComponent>("Bat");
}

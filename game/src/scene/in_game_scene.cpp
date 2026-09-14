#include <SDL_render.h>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/components/animation_component.hpp>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/asset_paths.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/render_overlay.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/debug_system.hpp>
#include <engine/widgets/widget.hpp>
#include <functional>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/geometry.hpp>
#include <game/play_state.hpp>
#include <game/rng.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/hud_widget.hpp>
#include <memory>
#include <numbers>
#include <random>

using namespace de;

InGameScene::InGameScene() = default;

void InGameScene::fail(entt::registry& registry, const std::string& reason)
{
    registry.ctx().emplace<StartupError>(StartupError{"level: " + reason});
}

void InGameScene::onEnter(entt::registry& registry)
{
    const auto& config = registry.ctx().get<Config>();
    const auto& assets = registry.ctx().get<AssetPaths>();

    // A new run: reset the state GamePlugin installed, rather than creating it.
    registry.ctx().get<GameState>() = GameState{};
    setPlayState(registry, PlayState::Playing);

    auto level = config.levels.find("arena");
    if (level == config.levels.end())
    {
        fail(registry, "game.json declares no level named 'arena'");
        return;
    }

    TiledLoader loader(&m_entities);
    if (auto loaded = loader.loadLevel(registry, assets.resolve(level->second));
        !loaded)
    {
        fail(registry, loaded.error());
        return;
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

    auto& textures = registry.ctx().get<TextureCache>();
    // Horizontal strips: slime 2x20, bat 3x20. AnimationComponent walks
    // columns.
    textures.load("slime", assets.resolve("images/slime.png").string());
    textures.load("bat", assets.resolve("images/bat.png").string());
    textures.load("bg-pattern",
                  assets.resolve("images/background-pattern.png").string());

    if (auto* audio = registry.ctx().find<AudioManager>(); audio != nullptr)
    {
        const auto& settings = registry.ctx().get<AudioSettings>();
        audio->setMusicVolume(settings.musicPercent);
        audio->setSfxVolume(settings.sfxPercent);
        audio->loadSound("bounce", assets.resolve("Audio/bounce.wav").string());
        audio->loadSound("collect",
                         assets.resolve("Audio/collect.wav").string());
        audio->loadSound("ui", assets.resolve("Audio/ui.wav").string());
        // Theme is started on Title and must keep playing; only start it if
        // something entered InGame without going through Title (e.g. --level).
        audio->loadMusic("theme", assets.resolve("Audio/theme.ogg").string());
        if (!audio->isMusicPlaying())
        {
            audio->playMusic("theme", true);
        }
    }

    tagObjectsByType(registry);

    // If the map had no Player object, spawn at the room centre.
    if (registry.view<PlayerComponent>().empty())
    {
        spawnSnake(
            registry,
            state.roomBounds.getLeft() + state.roomBounds.getWidth() * 0.5f,
            state.roomBounds.getTop() + state.roomBounds.getHeight() * 0.5f);
    }

    spawnBat(registry);

    const float mapWidth =
        static_cast<float>(loader.getWidth() * loader.getTileWidth());
    const float mapHeight =
        static_cast<float>(loader.getHeight() * loader.getTileHeight());
    initializeCamera(registry, config);
    initializeQuadtrees(registry, mapWidth, mapHeight);
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
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

void InGameScene::tagObjectsByType(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    const float tile = static_cast<float>(state.tileWidth);

    auto view = registry.view<ObjectTypeComponent, TransformComponent>();
    for (auto entity : view)
    {
        const auto& objectType = view.get<ObjectTypeComponent>(entity);
        if (objectType.type == "Player")
        {
            const auto& pos = view.get<TransformComponent>(entity).position;
            if (registry.valid(entity))
            {
                // Snap to the centre of the tile the object sits in so stride
                // steps stay flush with the wall cells.
                const int col = static_cast<int>(std::floor(pos.getX() / tile));
                const int row = static_cast<int>(std::floor(pos.getY() / tile));
                const Vector2D<float> center(
                    static_cast<float>(col) * tile + tile * 0.5f,
                    static_cast<float>(row) * tile + tile * 0.5f);

                registry.emplace_or_replace<PlayerComponent>(entity);
                SnakeComponent snake;
                snake.stride = tile;
                SlimeSegment head;
                head.at = center;
                head.to = center;
                head.direction = Vector2D<float>(1.0f, 0.0f);
                snake.segments.push_back(head);
                snake.nextDirection = head.direction;
                registry.emplace_or_replace<SnakeComponent>(entity, snake);
                registry.emplace_or_replace<DimensionComponent>(entity, tile,
                                                                tile);
                registry.emplace_or_replace<TextureComponent>(entity, "slime");
                registry.emplace_or_replace<SpriteComponent>(entity, 0, 0, 0);
                AnimationComponent anim;
                anim.totalFrames = 2;
                anim.animationTime = 0.2f;
                registry.emplace_or_replace<AnimationComponent>(entity, anim);
                registry.emplace_or_replace<TransformComponent>(
                    entity, Vector2D<float>(center.getX() - tile * 0.5f,
                                            center.getY() - tile * 0.5f));
            }
        }
    }
}

void InGameScene::spawnSnake(entt::registry& registry, float x, float y)
{
    auto entity = registry.create();
    registry.emplace<PlayerComponent>(entity);
    SnakeComponent snake;
    SlimeSegment head;
    head.at = Vector2D<float>(x, y);
    head.to = head.at;
    head.direction = Vector2D<float>(1.0f, 0.0f);
    snake.segments.push_back(head);
    snake.nextDirection = head.direction;
    registry.emplace<SnakeComponent>(entity, snake);
    registry.emplace<TransformComponent>(
        entity, Vector2D<float>(x - game::kSegmentSize * 0.5f,
                                y - game::kSegmentSize * 0.5f));
    registry.emplace<DimensionComponent>(entity, game::kSegmentSize,
                                         game::kSegmentSize);
    registry.emplace<TextureComponent>(entity, "slime");
    registry.emplace<SpriteComponent>(entity, 0, 0, 0);
    AnimationComponent anim;
    anim.totalFrames = 2;
    anim.animationTime = 0.2f;
    registry.emplace<AnimationComponent>(entity, anim);
    m_entities.push_back(entity);
}

void InGameScene::spawnBat(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    auto& rng = registry.ctx().get<GameRng>().engine;
    std::uniform_real_distribution<float> dx(state.roomBounds.getLeft(),
                                             state.roomBounds.getRight() -
                                                 game::kSegmentSize);
    std::uniform_real_distribution<float> dy(state.roomBounds.getTop(),
                                             state.roomBounds.getBottom() -
                                                 game::kSegmentSize);
    std::uniform_real_distribution<float> angleDist(
        0.0f, 2.0f * std::numbers::pi_v<float>);

    auto entity = registry.create();
    BatComponent bat;
    const float angle = angleDist(rng);
    bat.velocity =
        Vector2D<float>(std::cos(angle), std::sin(angle)) * bat.speed;
    registry.emplace<BatComponent>(entity, bat);
    registry.emplace<TransformComponent>(entity,
                                         Vector2D<float>(dx(rng), dy(rng)));
    registry.emplace<DimensionComponent>(entity, game::kSegmentSize,
                                         game::kSegmentSize);
    registry.emplace<TextureComponent>(entity, "bat");
    registry.emplace<SpriteComponent>(entity, 0, 0, 0);
    AnimationComponent anim;
    anim.totalFrames = 3;
    anim.animationTime = 0.1f;
    registry.emplace<AnimationComponent>(entity, anim);
    // No VelocityComponent: BatSystem integrates against roomBounds itself.
    m_entities.push_back(entity);
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

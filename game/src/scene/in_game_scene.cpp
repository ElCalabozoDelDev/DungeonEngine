#include <SDL_render.h>
#include <engine/audio/audio_manager.hpp>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/follow_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/solid_body_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/asset_paths.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/render_bottom.hpp>
#include <engine/graphics/render_collision.hpp>
#include <engine/graphics/render_object.hpp>
#include <engine/graphics/render_overlay.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/spatial/visibility_map.hpp>
#include <engine/systems/debug_system.hpp>
#include <engine/widgets/widget.hpp>
#include <functional>
#include <game/components/attack_component.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/objective_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/components/stairs_component.hpp>
#include <game/debug/player_editor.hpp>
#include <game/proc/dungeon_generator.hpp>
#include <game/proc/level_assembler.hpp>
#include <game/run/run_config.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/counter_widget.hpp>
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
    m_debugOpen = config.debug;

    if (!registry.ctx().contains<RunConfig>())
    {
        registry.ctx().emplace<RunConfig>();
    }

    registry.ctx().insert_or_assign<GameState>(GameState{});
    registry.ctx().get<Paused>().value = false;

    if (auto* audio = registry.ctx().find<AudioManager>(); audio != nullptr)
    {
        const auto& assets = registry.ctx().get<AssetPaths>();
        audio->loadSound("pickup", assets.resolve("Audio/pickup.wav").string());
        audio->loadSound("hurt", assets.resolve("Audio/hurt.wav").string());
    }

    if (!loadFloor(registry, 1, config, false))
    {
        return;
    }
}

void InGameScene::onUpdate(entt::registry& registry)
{
    auto* state = registry.ctx().find<GameState>();
    if (state == nullptr || state->pendingFloorChange == 0)
    {
        return;
    }

    const int nextFloor = state->pendingFloorChange;
    state->pendingFloorChange = 0;
    const auto& config = registry.ctx().get<Config>();
    loadFloor(registry, nextFloor, config, true);
}

void InGameScene::onExit(entt::registry& registry)
{
    clearLevelEntities(registry);
    if (auto* visibility = registry.ctx().find<VisibilityMap>();
        visibility != nullptr)
    {
        visibility->enabled = false;
    }
}

void InGameScene::clearLevelEntities(entt::registry& registry)
{
    if (registry.ctx().contains<SpatialIndex>())
    {
        registry.ctx().get<SpatialIndex>().clear();
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

bool InGameScene::loadFloor(entt::registry& registry, int floorIndex,
                            const Config& config, bool preserveRun)
{
    HealthComponent savedHealth;
    bool hadHealth = false;
    bool hadObjective = false;
    int itemsCollected = 0;
    int entranceColumn = 0;
    int entranceRow = 0;
    bool entranceRecorded = false;

    if (preserveRun)
    {
        auto* state = registry.ctx().find<GameState>();
        if (state != nullptr)
        {
            hadObjective = state->hasObjective;
            itemsCollected = state->itemsCollected;
            entranceColumn = state->entranceColumn;
            entranceRow = state->entranceRow;
            entranceRecorded = state->entranceRecorded;
        }
        auto players = registry.view<PlayerComponent, HealthComponent>();
        for (auto player : players)
        {
            savedHealth = players.get<HealthComponent>(player);
            hadHealth = true;
            break;
        }
        clearLevelEntities(registry);
    }

    auto& run = registry.ctx().get<RunConfig>();
    const auto& assets = registry.ctx().get<AssetPaths>();

    DungeonGenerator generator;
    const FloorBlueprint blueprint = generator.generate(run, floorIndex);

    LevelAssembler assembler(&m_entities);
    if (auto assembled = assembler.assemble(registry, blueprint, assets);
        !assembled)
    {
        fail(registry, assembled.error());
        return false;
    }

    auto& state = registry.ctx().get<GameState>();
    if (!preserveRun)
    {
        state = GameState{};
    }
    else
    {
        state.hasObjective = hadObjective;
        state.itemsCollected = itemsCollected;
        state.entranceColumn = entranceColumn;
        state.entranceRow = entranceRow;
        state.entranceRecorded = entranceRecorded;
        state.itemsTotal = 0;
    }
    state.currentFloor = floorIndex;
    state.pendingFloorChange = 0;
    state.gameOver = false;
    state.victory = false;

    tagObjectsByType(registry);

    if (preserveRun && hadHealth)
    {
        auto players = registry.view<PlayerComponent, HealthComponent>();
        for (auto player : players)
        {
            players.get<HealthComponent>(player) = savedHealth;
        }
    }

    if (floorIndex == 1 && !state.entranceRecorded)
    {
        auto players = registry.view<PlayerComponent, TransformComponent>();
        for (auto player : players)
        {
            const auto& pos = players.get<TransformComponent>(player).position;
            state.entranceColumn = static_cast<int>(
                (pos.getX() + run.tileSize * 0.5f) / run.tileSize);
            state.entranceRow = static_cast<int>(
                (pos.getY() + run.tileSize * 0.5f) / run.tileSize);
            state.entranceRecorded = true;
            break;
        }
    }

    initializeVisibility(registry, blueprint.columns, blueprint.rows,
                         blueprint.tileSize);
    for (int row = 0; row < blueprint.rows; ++row)
    {
        for (int col = 0; col < blueprint.columns; ++col)
        {
            const bool opaque =
                blueprint.at(col, row) != FloorBlueprint::Cell::Floor;
            registry.ctx().get<VisibilityMap>().setOpaque(col, row, opaque);
        }
    }

    const int mapWidth = assembler.mapPixelWidth();
    const int mapHeight = assembler.mapPixelHeight();
    initializeCamera(registry, mapWidth, mapHeight, config);
    initializeQuadtrees(registry, static_cast<float>(mapWidth),
                        static_cast<float>(mapHeight));
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
    initializeRenderers(registry);
    if (!preserveRun)
    {
        initializeHud(registry);
        initializeDebug(registry, m_debugOpen);
    }
    else
    {
        initializeHud(registry);
        initializeDebug(registry, m_debugOpen);
    }
    return true;
}

void InGameScene::initializeVisibility(entt::registry& registry, int columns,
                                       int rows, float tileSize)
{
    if (!registry.ctx().contains<VisibilityMap>())
    {
        registry.ctx().emplace<VisibilityMap>();
    }
    registry.ctx().get<VisibilityMap>().reset(columns, rows, tileSize);
}

void InGameScene::tagObjectsByType(entt::registry& registry)
{
    auto& state = registry.ctx().get<GameState>();

    auto view = registry.view<ObjectTypeComponent>();
    for (auto entity : view)
    {
        const auto& objectType = view.get<ObjectTypeComponent>(entity);

        if (objectType.type == "Player")
        {
            registry.emplace<PlayerComponent>(entity);
            registry.emplace<SpeedComponent>(entity);
            registry.emplace<HealthComponent>(entity);
            registry.emplace<AttackComponent>(entity);
            registry.emplace<SolidBodyComponent>(entity);
        }
        else if (objectType.type == "Enemy")
        {
            registry.emplace<EnemyComponent>(entity);
            registry.emplace<SolidBodyComponent>(entity);
            HealthComponent enemyHealth;
            enemyHealth.current = EnemyComponent{}.maxHealth;
            enemyHealth.max = EnemyComponent{}.maxHealth;
            registry.emplace<HealthComponent>(entity, enemyHealth);
        }
        else if (objectType.type == "Item")
        {
            registry.emplace<ItemComponent>(entity);
            ++state.itemsTotal;
        }
        else if (objectType.type == "Objective")
        {
            registry.emplace<ObjectiveComponent>(entity);
        }
        else if (objectType.type == "StairsDown")
        {
            registry.emplace<StairsComponent>(
                entity, StairsComponent{StairsDirection::Down});
        }
        else if (objectType.type == "StairsUp")
        {
            registry.emplace<StairsComponent>(
                entity, StairsComponent{StairsDirection::Up});
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

void InGameScene::initializeHud(entt::registry& registry)
{
    auto hudEntity = registry.create();
    registry.emplace<Widget>(hudEntity, std::make_unique<HudWidget>());
    m_entities.push_back(hudEntity);
}

void InGameScene::initializeDebug(entt::registry& registry, bool open)
{
    if (!open)
        return;

    auto& debugSystem = registry.ctx().get<DebugSystem>();
    debugSystem.setOpen(open);
    debugSystem.register_component<PlayerComponent>("Player");
    debugSystem.register_component<CameraComponent>("Camera");
    debugSystem.register_component<HealthComponent>("Health");
    debugSystem.register_component<EnemyComponent>("Enemy");

    auto counterEntity = registry.create();
    registry.emplace<Widget>(counterEntity, std::make_unique<CounterWidget>());
    m_entities.push_back(counterEntity);
}

#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/systems/camera_system.hpp>
#include <engine/systems/collision_system.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/run/run_config.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/menu_scene.hpp>
#include <game/state.hpp>
#include <game/systems/combat_system.hpp>
#include <game/systems/enemy_ai_system.hpp>
#include <game/systems/fov_system.hpp>
#include <game/systems/movement_system.hpp>
#include <game/systems/progression_system.hpp>
#include <memory>

using namespace de;

void GamePlugin::mount(de::GameLoop& gameLoop)
{
    auto debugSystem = std::make_shared<DebugSystem>();
    auto sceneSystem = std::make_shared<SceneSystem>();

    gameLoop.addSetupCallback(
        [sceneSystem, debugSystem](entt::registry& registry)
        {
            if (registry.ctx().contains<StartupError>())
            {
                return;
            }

            auto& actions = registry.ctx().get<ActionMap>();
            actions.bind("move_up", SDL_SCANCODE_UP);
            actions.bind("move_up", SDL_SCANCODE_W);
            actions.bind("move_down", SDL_SCANCODE_DOWN);
            actions.bind("move_down", SDL_SCANCODE_S);
            actions.bind("move_left", SDL_SCANCODE_LEFT);
            actions.bind("move_left", SDL_SCANCODE_A);
            actions.bind("move_right", SDL_SCANCODE_RIGHT);
            actions.bind("move_right", SDL_SCANCODE_D);
            actions.bind("attack", SDL_SCANCODE_J);
            actions.bind("attack", SDL_SCANCODE_SPACE);
            actions.bind("pause", SDL_SCANCODE_ESCAPE);
            actions.bind("confirm", SDL_SCANCODE_RETURN);
            actions.bind("reload_scene", SDL_SCANCODE_F5);

            registry.ctx().emplace<GameState>();
            registry.ctx().emplace<RunConfig>();
            registry.ctx().emplace<Paused>();

            registry.ctx().emplace<SceneSystem&>(*sceneSystem);
            registry.ctx().emplace<DebugSystem&>(*debugSystem);

            sceneSystem->setScene(registry, std::make_unique<MenuScene>());
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto* scenes = registry.ctx().find<SceneSystem>();
            auto* input = registry.ctx().find<InputState>();
            auto* actions = registry.ctx().find<ActionMap>();
            auto* paused = registry.ctx().find<Paused>();
            auto* state = registry.ctx().find<GameState>();
            if (scenes == nullptr || input == nullptr || actions == nullptr ||
                paused == nullptr || state == nullptr)
            {
                return;
            }

            if (state->gameOver || state->victory)
            {
                scenes->requestScene(std::make_unique<MenuScene>());
                state->gameOver = false;
                state->victory = false;
                return;
            }

            if (actions->wasPressed(*input, "pause"))
            {
                paused->value = !paused->value;
            }

            if (actions->wasPressed(*input, "reload_scene"))
            {
                scenes->requestScene(std::make_unique<InGameScene>());
            }
        });

    gameLoop.addFixedSystem(std::make_shared<MovementSystem>());
    gameLoop.addFixedSystem(std::make_shared<EnemyAISystem>());
    gameLoop.addFixedSystem(std::make_shared<CollisionSystem>());
    gameLoop.addFixedSystem(std::make_shared<CombatSystem>());
    gameLoop.addFixedSystem(std::make_shared<ProgressionSystem>());

    gameLoop.addSystem(std::make_shared<FovSystem>());
    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

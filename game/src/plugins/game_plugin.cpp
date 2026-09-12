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
#include <game/scene/in_game_scene.hpp>
#include <game/scene/menu_scene.hpp>
#include <game/state.hpp>
#include <game/systems/combat_system.hpp>
#include <game/systems/enemy_ai_system.hpp>
#include <game/systems/movement_system.hpp>
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

            // This game's controls. Two bindings per action, so arrows and
            // WASD both work; rebinding is a change here, not in a system.
            auto& actions = registry.ctx().get<ActionMap>();
            actions.bind("move_up", SDL_SCANCODE_UP);
            actions.bind("move_up", SDL_SCANCODE_W);
            actions.bind("move_down", SDL_SCANCODE_DOWN);
            actions.bind("move_down", SDL_SCANCODE_S);
            actions.bind("move_left", SDL_SCANCODE_LEFT);
            actions.bind("move_left", SDL_SCANCODE_A);
            actions.bind("move_right", SDL_SCANCODE_RIGHT);
            actions.bind("move_right", SDL_SCANCODE_D);
            actions.bind("pause", SDL_SCANCODE_ESCAPE);
            actions.bind("confirm", SDL_SCANCODE_RETURN);
            actions.bind("confirm", SDL_SCANCODE_SPACE);
            actions.bind("reload_scene", SDL_SCANCODE_F5);

            registry.ctx().emplace<GameState>();
            registry.ctx().emplace<Paused>();

            // The loop owns these systems; the context holds a reference so
            // there is one owner instead of a second shared_ptr.
            registry.ctx().emplace<SceneSystem&>(*sceneSystem);
            registry.ctx().emplace<DebugSystem&>(*debugSystem);

            // Immediate, not requested: this runs during setup, where a
            // failure to load still has to reach GameLoop as a StartupError
            // before the first frame.
            sceneSystem->setScene(registry, std::make_unique<MenuScene>());
        });

    // Scene and pause transitions, in one place. Requests are deferred:
    // SceneSystem applies them between frames rather than tearing the level
    // down underneath the systems that have not run yet.
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

            if (state->gameOver)
            {
                scenes->requestScene(std::make_unique<MenuScene>());
                state->gameOver = false;
                return;
            }

            if (actions->wasPressed(*input, "pause"))
            {
                paused->value = !paused->value;
            }

            // F5 rebuilds the level: useful while editing a map, and it is
            // what exercises the deferred scene switch.
            if (actions->wasPressed(*input, "reload_scene"))
            {
                scenes->requestScene(std::make_unique<InGameScene>());
            }
        });

    // Fixed step, in order: input to velocity, enemy decisions, then the
    // engine pushes bodies out of walls, then this game's reactions to
    // whatever ended up touching. Integration and index sync run between
    // them, registered by BasePlugin.
    gameLoop.addFixedSystem(std::make_shared<MovementSystem>());
    gameLoop.addFixedSystem(std::make_shared<EnemyAISystem>());
    gameLoop.addFixedSystem(std::make_shared<CollisionSystem>());
    gameLoop.addFixedSystem(std::make_shared<CombatSystem>());

    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

#include <SDL.h>
#include <engine/core/startup_error.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/systems/camera_system.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/scene/in_game_scene.hpp>
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
            actions.bind("reload_scene", SDL_SCANCODE_F5);

            // The loop owns these systems; the context holds a reference so
            // there is one owner instead of a second shared_ptr.
            registry.ctx().emplace<SceneSystem&>(*sceneSystem);
            registry.ctx().emplace<DebugSystem&>(*debugSystem);

            // Immediate, not requested: this runs during setup, where a
            // failure to load still has to reach GameLoop as a StartupError
            // before the first frame.
            sceneSystem->setScene(registry, std::make_unique<InGameScene>());
        });

    // F5 rebuilds the level. Useful while iterating on a map, and it is what
    // exercises the deferred scene switch: the request is made here, at frame
    // begin, and applied by SceneSystem rather than tearing the level down
    // underneath the systems that have not run yet.
    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto* scenes = registry.ctx().find<SceneSystem>();
            auto* input = registry.ctx().find<InputState>();
            auto* actions = registry.ctx().find<ActionMap>();
            if (scenes == nullptr || input == nullptr || actions == nullptr)
            {
                return;
            }
            if (actions->wasPressed(*input, "reload_scene"))
            {
                scenes->requestScene(std::make_unique<InGameScene>());
            }
        });

    // Turning input into velocity advances with the fixed step, alongside the
    // integration it feeds. Event pumping itself belongs to the engine's
    // InputPlugin, not here.
    gameLoop.addFixedSystem(std::make_shared<MovementSystem>());

    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

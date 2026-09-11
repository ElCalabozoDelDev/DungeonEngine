#include <SDL.h>
#include <engine/core/startup_error.hpp>
#include <engine/input/action_map.hpp>
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

            registry.ctx().emplace<std::shared_ptr<SceneSystem>>(sceneSystem);
            registry.ctx().emplace<std::shared_ptr<DebugSystem>>(debugSystem);
            sceneSystem->changeScene(registry, std::make_unique<InGameScene>());
        });

    // Turning input into velocity advances with the fixed step, alongside the
    // integration it feeds. Event pumping itself belongs to the engine's
    // InputPlugin, not here.
    gameLoop.addFixedSystem(std::make_shared<MovementSystem>());

    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

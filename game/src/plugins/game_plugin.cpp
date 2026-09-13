#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/systems/camera_system.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/play_state.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/rng.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/systems/bat_system.hpp>
#include <game/systems/grayscale_fade_system.hpp>
#include <game/systems/snake_system.hpp>
#include <game/ui/bitmap_font.hpp>
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
            actions.bind("pause", SDL_SCANCODE_ESCAPE);
            actions.bind("confirm", SDL_SCANCODE_RETURN);
            actions.bind("confirm", SDL_SCANCODE_SPACE);
            actions.bind("reload_scene", SDL_SCANCODE_F5);

            registry.ctx().emplace<GameState>();
            registry.ctx().emplace<AudioSettings>();
            registry.ctx().emplace<Paused>();
            registry.ctx().emplace<GameRng>();

            registry.ctx().emplace<SceneSystem&>(*sceneSystem);
            registry.ctx().emplace<DebugSystem&>(*debugSystem);

            sceneSystem->setScene(registry, std::make_unique<TitleScene>());
        });

    // Destroy BMFont textures before SDL tears down the renderer.
    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            registry.ctx().erase<game::ui::BitmapFont>();
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto* scenes = registry.ctx().find<SceneSystem>();
            auto* input = registry.ctx().find<InputState>();
            auto* actions = registry.ctx().find<ActionMap>();
            auto* state = registry.ctx().find<GameState>();
            if (scenes == nullptr || input == nullptr || actions == nullptr ||
                state == nullptr)
            {
                return;
            }

            // Esc toggles pause while playing; UI owns GameOver transitions.
            // Fixed systems: SnakeSystem then BatSystem (eat after move).
            if (state->playState == PlayState::Playing ||
                state->playState == PlayState::Paused)
            {
                if (actions->wasPressed(*input, "pause"))
                {
                    if (state->playState == PlayState::Playing)
                    {
                        setPlayState(registry, PlayState::Paused);
                    }
                    else
                    {
                        setPlayState(registry, PlayState::Playing);
                    }
                }
            }

            if (actions->wasPressed(*input, "reload_scene"))
            {
                scenes->requestScene(std::make_unique<InGameScene>());
            }
        });

    gameLoop.addFixedSystem(std::make_shared<SnakeSystem>());
    gameLoop.addFixedSystem(std::make_shared<BatSystem>());

    gameLoop.addSystem(std::make_shared<CameraSystem>());
    gameLoop.addSystem(std::make_shared<GrayscaleFadeSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

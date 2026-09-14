#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/systems/debug_system.hpp>
#include <game/assets.hpp>
#include <game/play_state.hpp>
#include <game/plugins/game_plugin.hpp>
#include <game/rng.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/systems/bat_system.hpp>
#include <game/systems/grayscale_fade_system.hpp>
#include <game/systems/snake_system.hpp>
#include <game/systems/snake_view_system.hpp>
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

            // The game's shared resources, installed exactly once. Scenes,
            // systems and widgets read them with ctx().get and never create
            // them: a missing resource should fail loudly here, not be
            // quietly default-constructed wherever it was first needed.
            registry.ctx().emplace<GameState>();
            registry.ctx().emplace<AudioSettings>();
            registry.ctx().emplace<Paused>();
            registry.ctx().emplace<GameRng>();
            registry.ctx().emplace<WorldColorGrade>();

            registry.ctx().emplace<SceneSystem&>(*sceneSystem);
            registry.ctx().emplace<DebugSystem&>(*debugSystem);

            game::loadGameAssets(registry);
            sceneSystem->setScene(registry, std::make_unique<TitleScene>());
        });

    // Destroy BMFont textures before SDL tears down the renderer.
    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        { registry.ctx().erase<game::ui::BitmapFont>(); });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            // Frames only run after a successful setup, which installed all
            // of these.
            auto& scenes = registry.ctx().get<SceneSystem>();
            const auto& input = registry.ctx().get<InputState>();
            const auto& actions = registry.ctx().get<ActionMap>();
            const auto& state = registry.ctx().get<GameState>();

            // Esc toggles pause while playing; UI owns GameOver transitions.
            // Fixed systems: SnakeSystem then BatSystem (eat after move).
            if (state.playState == PlayState::Playing ||
                state.playState == PlayState::Paused)
            {
                if (actions.wasPressed(input, "pause"))
                {
                    if (state.playState == PlayState::Playing)
                    {
                        setPlayState(registry, PlayState::Paused);
                    }
                    else
                    {
                        setPlayState(registry, PlayState::Playing);
                    }
                }
            }

            if (actions.wasPressed(input, "reload_scene"))
            {
                scenes.requestScene(std::make_unique<InGameScene>());
            }
        });

    gameLoop.addFixedSystem(std::make_shared<SnakeSystem>());
    gameLoop.addFixedSystem(std::make_shared<BatSystem>());
    // Presentation after the simulation it draws, still inside the step so
    // the spatial index sync that ends it sees the sprites' final positions.
    gameLoop.addFixedSystem(std::make_shared<SnakeViewSystem>());

    gameLoop.addSystem(std::make_shared<GrayscaleFadeSystem>());
    gameLoop.addSystem(sceneSystem);
    gameLoop.addSystem(debugSystem);
}

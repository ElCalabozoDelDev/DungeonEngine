#include <SDL.h>
#include <array>
#include <engine/components/transform_component.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/rng.hpp>
#include <game/sim/sim_plugin.hpp>
#include <game/state.hpp>
#include <iomanip>
#include <iostream>
#include <limits>

using namespace de;

SimPlugin::SimPlugin(SimOptions options) : m_options(std::move(options)) {}

void SimPlugin::mount(de::GameLoop& gameLoop)
{
    if (!m_options.window)
    {
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 0);
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 0);
    }

    gameLoop.addSetupCallback(
        [this](entt::registry& registry)
        {
            if (!registry.ctx().contains<GameRng>())
            {
                registry.ctx().emplace<GameRng>();
            }
            registry.ctx().get<GameRng>().engine.seed(m_options.seed);

            m_csv.open(m_options.csvPath);
            if (!m_csv)
            {
                std::cerr << "sim: cannot write " << m_options.csvPath << "\n";
                m_failed = true;
                if (auto* flow = registry.ctx().find<ControlFlow>())
                {
                    *flow = ControlFlow::Exit;
                }
                return;
            }
            // Positions are printed with enough digits to round-trip a float,
            // so a before/after diff catches any change in the trajectories,
            // not just in the score.
            m_csv << std::setprecision(
                std::numeric_limits<float>::max_digits10);
            m_csv << "step,score,length,game_over,head_x,head_y,bat_x,bat_y\n";
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto* input = registry.ctx().find<InputState>();
            auto* actions = registry.ctx().find<ActionMap>();
            if (input == nullptr || actions == nullptr)
            {
                return;
            }

            std::array<Uint8, SDL_NUM_SCANCODES> keys{};
            for (SDL_Scancode key : actions->keysFor("move_right"))
            {
                keys[static_cast<std::size_t>(key)] = 1;
            }
            input->setKeyboard(keys.data(), SDL_NUM_SCANCODES);
        });

    gameLoop.addFrameEndCallback(
        [this](entt::registry& registry)
        {
            if (m_failed || !m_csv)
            {
                return;
            }

            ++m_steps;
            int score = 0;
            int length = 0;
            bool over = false;
            if (const auto* state = registry.ctx().find<GameState>())
            {
                score = state->score;
                over = state->playState == PlayState::GameOver;
            }
            Vector2D<float> head;
            for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
            {
                const auto& snake = registry.get<SnakeComponent>(entity);
                length = static_cast<int>(snake.segments.size());
                if (!snake.segments.empty())
                {
                    head = snake.segments.front().to;
                }
            }
            Vector2D<float> bat;
            for (auto entity :
                 registry.view<BatComponent, TransformComponent>())
            {
                bat = registry.get<TransformComponent>(entity).position;
            }

            m_csv << m_steps << ',' << score << ',' << length << ','
                  << (over ? 1 : 0) << ',' << head.getX() << ',' << head.getY()
                  << ',' << bat.getX() << ',' << bat.getY() << '\n';

            if (over || m_steps >= m_options.maxSteps)
            {
                std::cout << "sim: outcome=" << (over ? "game_over" : "timeout")
                          << " score=" << score << " length=" << length
                          << " steps=" << m_steps << '\n';
                if (auto* flow = registry.ctx().find<ControlFlow>())
                {
                    *flow = ControlFlow::Exit;
                }
            }
        });
}

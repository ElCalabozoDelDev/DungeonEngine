#include <SDL.h>
#include <array>
#include <engine/core/game_loop.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/sim/sim_plugin.hpp>
#include <game/state.hpp>
#include <iostream>

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
            m_csv << "step,score,length,game_over\n";
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
                over =
                    state->gameOver || state->playState == PlayState::GameOver;
            }
            for (auto entity : registry.view<PlayerComponent, SnakeComponent>())
            {
                length = static_cast<int>(
                    registry.get<SnakeComponent>(entity).segments.size());
            }

            m_csv << m_steps << ',' << score << ',' << length << ','
                  << (over ? 1 : 0) << '\n';

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

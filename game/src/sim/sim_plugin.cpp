#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <game/components/player_component.hpp>
#include <game/sim/sim_plugin.hpp>
#include <iostream>

using namespace de;

namespace
{
/// Frames the menu is allowed to take before the level appears. The level is
/// requested in a setup callback and applied by SceneSystem mid-frame, so a
/// few frames without a player are normal; three hundred are not.
constexpr int MaxFramesWithoutPlayer = 300;

} // namespace

SimPlugin::SimPlugin(SimOptions options) : m_options(std::move(options)) {}

bool SimPlugin::cleared() const
{
    return m_sampler != nullptr && m_sampler->summary().outcome == "cleared";
}

void SimPlugin::mount(de::GameLoop& gameLoop)
{
    // mount() runs at addPlugin() time and SDLPlugin only *registers* a setup
    // callback, so SDL is not up yet and these still take effect. Set with
    // overwrite = 0: a CTest ENVIRONMENT entry, or a user who wants to watch,
    // both win over this default.
    if (!m_options.window)
    {
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 0);
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);
        SDL_setenv("SDL_RENDER_DRIVER", "software", 0);
    }

    m_policy.setSeed(m_options.seed);
    m_sampler =
        std::make_shared<TelemetrySampler>(m_policy, m_options.maxSteps);

    gameLoop.addSetupCallback(
        [this](entt::registry& registry)
        {
            m_csv.open(m_options.csvPath);
            if (!m_csv)
            {
                std::cerr << "sim: cannot write " << m_options.csvPath << "\n";
                m_failed = true;
                // Not a StartupError: that would abort before teardown and
                // would report a level-loading problem, which this is not.
                if (auto* flow = registry.ctx().find<ControlFlow>())
                {
                    *flow = ControlFlow::Exit;
                }
                return;
            }
            m_writer = std::make_unique<TelemetryWriter>(m_csv);
            m_writer->writeHeader();
            m_sampler->setWriter(m_writer.get());
        });

    gameLoop.addFrameBeginCallback(
        [this](entt::registry& registry)
        {
            m_policy.update(registry);

            if (m_sampler->finished())
            {
                return;
            }
            const auto players = registry.view<PlayerComponent>();
            if (players.begin() != players.end())
            {
                m_framesWithoutPlayer = 0;
                return;
            }
            if (++m_framesWithoutPlayer >= MaxFramesWithoutPlayer)
            {
                std::cerr << "sim: no player entity after "
                          << MaxFramesWithoutPlayer << " frames\n";
                m_failed = true;
                if (auto* flow = registry.ctx().find<ControlFlow>())
                {
                    *flow = ControlFlow::Exit;
                }
            }
        });

    // Last in the fixed step, so it observes post-collision, post-damage,
    // post-pickup state.
    gameLoop.addFixedSystem(m_sampler);

    gameLoop.addTeardownCallback(
        [this](entt::registry&)
        {
            SimSummary summary = m_sampler->summary();
            if (!m_sampler->finished() && summary.steps > 0)
            {
                // Something else ended the loop first -- --frames, or a quit.
                summary.outcome = "interrupted";
            }
            summary.seed = m_options.seed;
            summary.csvPath = m_options.csvPath;

            m_csv.flush();
            m_csv.close();

            const std::string text = TelemetryWriter::summary(summary);
            std::cout << text << std::flush;
            if (!m_options.summaryPath.empty())
            {
                std::ofstream out(m_options.summaryPath);
                if (out)
                {
                    out << text;
                }
                else
                {
                    std::cerr << "sim: cannot write " << m_options.summaryPath
                              << "\n";
                }
            }
        });
    // Teardown callbacks are connected front-first, so the last plugin
    // mounted tears down first: the files are written before SDL_Quit.
}

#ifndef GAME_SIM_SIM_PLUGIN_HPP
#define GAME_SIM_SIM_PLUGIN_HPP

#include <engine/plugins/plugin.hpp>
#include <fstream>
#include <game/sim/seek_items_policy.hpp>
#include <game/sim/sim_options.hpp>
#include <game/sim/telemetry_sampler.hpp>
#include <game/sim/telemetry_writer.hpp>
#include <memory>

/// Turns a normal run into a measured one: scripted input in, CSV out.
///
/// Mount it *after* GamePlugin. Frame-begin callbacks fire in registration
/// order, so the policy's injection then lands after InputPlugin has written
/// the real keyboard and overwrites it -- which is the whole mechanism.
class SimPlugin final : public de::Plugin
{
public:
    explicit SimPlugin(SimOptions options);

    void mount(de::GameLoop& gameLoop) override;

    /// An infrastructure problem, not a gameplay outcome: the CSV could not
    /// be written, or the level never produced a player. Dying is not a
    /// failure -- a balance harness must not go red because a level is hard.
    bool failed() const { return m_failed; }

    bool cleared() const;

private:
    SimOptions m_options;
    SeekItemsPolicy m_policy;
    std::shared_ptr<TelemetrySampler> m_sampler;
    std::ofstream m_csv;
    std::unique_ptr<TelemetryWriter> m_writer;
    bool m_failed = false;
    int m_framesWithoutPlayer = 0;
};

#endif // GAME_SIM_SIM_PLUGIN_HPP

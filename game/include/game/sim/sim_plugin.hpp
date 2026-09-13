#ifndef GAME_SIM_SIM_PLUGIN_HPP
#define GAME_SIM_SIM_PLUGIN_HPP

#include <engine/plugins/plugin.hpp>
#include <fstream>
#include <game/sim/sim_options.hpp>

/// Minimal snake sim: always-right policy, CSV with score/length/steps.
class SimPlugin final : public de::Plugin
{
public:
    explicit SimPlugin(SimOptions options);

    void mount(de::GameLoop& gameLoop) override;

    bool failed() const { return m_failed; }

private:
    SimOptions m_options;
    std::ofstream m_csv;
    bool m_failed = false;
    int m_steps = 0;
};

#endif // GAME_SIM_SIM_PLUGIN_HPP

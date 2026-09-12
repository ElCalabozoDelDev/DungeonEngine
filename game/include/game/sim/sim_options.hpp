#ifndef GAME_SIM_SIM_OPTIONS_HPP
#define GAME_SIM_SIM_OPTIONS_HPP

#include <string>

/// How a headless telemetry run is configured. Filled from the command line
/// in main.cpp and handed to SimPlugin.
struct SimOptions
{
    /// Where the per-step CSV goes.
    std::string csvPath = "sim.csv";

    /// Where the end-of-run summary also goes. Empty means stdout only.
    std::string summaryPath;

    /// Fixed-step budget. 3600 is sixty simulated seconds.
    int maxSteps = 3600;

    /// Order in which items are visited. 0 is strict nearest-first and is the
    /// reproducible default; any other value shuffles the order so a run can
    /// explore a different route through the level.
    unsigned int seed = 0;

    /// Leave SDL's drivers alone instead of forcing the dummy ones, so the run
    /// can be watched in a real window.
    bool window = false;

    /// Exit non-zero unless every item was collected.
    bool requireClear = false;
};

#endif // GAME_SIM_SIM_OPTIONS_HPP

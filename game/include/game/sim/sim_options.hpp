#ifndef GAME_SIM_SIM_OPTIONS_HPP
#define GAME_SIM_SIM_OPTIONS_HPP

#include <string>

struct SimOptions
{
    std::string csvPath = "sim.csv";
    std::string summaryPath;
    int maxSteps = 3600;
    unsigned int seed = 0;
    bool window = false;
    bool requireClear = false;
};

#endif // GAME_SIM_SIM_OPTIONS_HPP

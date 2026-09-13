#ifndef GAME_RNG_HPP
#define GAME_RNG_HPP

#include <random>

/// Shared gameplay RNG. Seeded from SimOptions::seed in --sim; otherwise
/// seeded from std::random_device for interactive play.
struct GameRng
{
    std::mt19937 engine{std::random_device{}()};
};

#endif // GAME_RNG_HPP

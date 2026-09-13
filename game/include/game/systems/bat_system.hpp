#ifndef GAME_SYSTEMS_BAT_SYSTEM_HPP
#define GAME_SYSTEMS_BAT_SYSTEM_HPP

#include <engine/core/vector_2d.hpp>
#include <engine/systems/system.hpp>
#include <random>

namespace game::bat
{
/// A velocity of `speed` in a uniformly random direction. Draws exactly one
/// number from `rng`.
de::Vector2D<float> randomVelocity(float speed, std::mt19937& rng);

} // namespace game::bat

/// Flies the bats around the room, bouncing off its walls, and lets the snake
/// eat them. A fixed system, run after SnakeSystem; it does nothing once the
/// run is over (see PlayState).
class BatSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_BAT_SYSTEM_HPP

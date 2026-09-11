#ifndef GAME_SYSTEMS_MOVEMENT_SYSTEM_HPP
#define GAME_SYSTEMS_MOVEMENT_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

/// Turns keyboard state into velocity for the player entity.
class MovementSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_MOVEMENT_SYSTEM_HPP

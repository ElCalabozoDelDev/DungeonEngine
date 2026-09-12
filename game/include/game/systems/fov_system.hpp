#ifndef GAME_SYSTEMS_FOV_SYSTEM_HPP
#define GAME_SYSTEMS_FOV_SYSTEM_HPP

#include <engine/systems/system.hpp>

/// Recomputes libtcod FOV from the player and updates de::VisibilityMap.
class FovSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_FOV_SYSTEM_HPP

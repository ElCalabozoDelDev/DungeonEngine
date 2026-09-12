#ifndef GAME_SYSTEMS_PROGRESSION_SYSTEM_HPP
#define GAME_SYSTEMS_PROGRESSION_SYSTEM_HPP

#include <engine/systems/system.hpp>

/// Stairs overlap, objective pickup side-effects, and entrance win check.
class ProgressionSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_PROGRESSION_SYSTEM_HPP

#ifndef GAME_SYSTEMS_BAT_SYSTEM_HPP
#define GAME_SYSTEMS_BAT_SYSTEM_HPP

#include <engine/systems/system.hpp>

class BatSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_BAT_SYSTEM_HPP

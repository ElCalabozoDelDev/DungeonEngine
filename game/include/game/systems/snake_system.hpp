#ifndef GAME_SYSTEMS_SNAKE_SYSTEM_HPP
#define GAME_SYSTEMS_SNAKE_SYSTEM_HPP

#include <engine/systems/system.hpp>

class SnakeSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_SNAKE_SYSTEM_HPP

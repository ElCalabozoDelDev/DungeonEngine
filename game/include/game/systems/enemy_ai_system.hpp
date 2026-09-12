#ifndef GAME_SYSTEMS_ENEMY_AI_SYSTEM_HPP
#define GAME_SYSTEMS_ENEMY_AI_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

/// Enemies walk towards the player once it is close enough, and stand still
/// otherwise. Deliberately the simplest thing that makes the level feel alive.
class EnemyAISystem final : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_ENEMY_AI_SYSTEM_HPP

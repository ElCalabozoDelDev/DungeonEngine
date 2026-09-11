#ifndef GAME_SYSTEMS_COMBAT_SYSTEM_HPP
#define GAME_SYSTEMS_COMBAT_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

/// What happens when the player touches something: enemies hurt, items are
/// collected.
///
/// Solid collision against the level is the engine's job (CollisionSystem);
/// this is the part that is specific to this game, which is why the two are
/// separate systems rather than the single one the prototype had.
class CombatSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_COMBAT_SYSTEM_HPP

#ifndef GAME_COMPONENTS_ATTACK_COMPONENT_HPP
#define GAME_COMPONENTS_ATTACK_COMPONENT_HPP

/// Player (or future actor) offensive capability.
///
/// Shape is intentionally generic: range + cooldown + damage can describe a
/// melee swing today and feed a projectile spawn later without rewriting the
/// health/death pipeline.
struct AttackComponent
{
    int damage = 1;
    float range = 24.0f;
    float cooldownSeconds = 0.35f;
    float cooldownRemaining = 0.0f;
};

#endif // GAME_COMPONENTS_ATTACK_COMPONENT_HPP

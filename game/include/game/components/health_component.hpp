#ifndef GAME_COMPONENTS_HEALTH_COMPONENT_HPP
#define GAME_COMPONENTS_HEALTH_COMPONENT_HPP

/// Hit points, and the cooldown that stops contact damage applying every
/// single fixed step while two bodies stay overlapped.
struct HealthComponent
{
    int current = 5;
    int max = 5;
    float invulnerabilityFor = 0.0f;
};

#endif // GAME_COMPONENTS_HEALTH_COMPONENT_HPP

#ifndef GAME_COMPONENTS_SPEED_COMPONENT_HPP
#define GAME_COMPONENTS_SPEED_COMPONENT_HPP

/// How fast an input-driven entity moves, in pixels per second.
/// Previously this was the literal 200.0f, repeated four times inside
/// MovementSystem.
struct SpeedComponent
{
    float value = 200.0f;
};

#endif // GAME_COMPONENTS_SPEED_COMPONENT_HPP

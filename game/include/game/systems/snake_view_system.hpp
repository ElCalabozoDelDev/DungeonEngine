#ifndef GAME_SYSTEMS_SNAKE_VIEW_SYSTEM_HPP
#define GAME_SYSTEMS_SNAKE_VIEW_SYSTEM_HPP

#include <engine/systems/system.hpp>

/// Places the snake's sprites where SnakeSystem left its segments: the head
/// entity's own sprite for segment 0, one sprite entity per body segment.
/// Creates and destroys body sprites as the snake grows, and keeps them filed
/// in the spatial index.
///
/// A fixed system rather than a frame one, run after the gameplay systems:
/// the spatial index is re-synced at the end of each fixed step, and sprites
/// moved outside a step would stay misfiled until the next one.
class SnakeViewSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_SNAKE_VIEW_SYSTEM_HPP

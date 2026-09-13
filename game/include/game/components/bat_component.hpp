#ifndef GAME_COMPONENTS_BAT_COMPONENT_HPP
#define GAME_COMPONENTS_BAT_COMPONENT_HPP

#include <engine/core/vector_2d.hpp>

/// Bouncing collectible. Owns its own velocity so TransformSystem (level-edge
/// clamp + frame delta) does not fight roomBounds bounce.
struct BatComponent
{
    /// Pixels per second in logical space (~MonoGame's 5 px/frame at 60 Hz
    /// scaled from window to logical).
    float speed = 75.0f;

    de::Vector2D<float> velocity{75.0f, 0.0f};
};

#endif // GAME_COMPONENTS_BAT_COMPONENT_HPP

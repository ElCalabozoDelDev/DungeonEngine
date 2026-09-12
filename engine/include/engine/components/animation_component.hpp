#ifndef DE_COMPONENTS_ANIMATION_COMPONENT_HPP
#define DE_COMPONENTS_ANIMATION_COMPONENT_HPP

#include <cstdint>

namespace de
{
/// What an animation does when it runs out of frames.
enum class AnimationLoop : std::uint8_t
{
    /// Back to the first frame and round again.
    Loop,
    /// Forwards to the end, then backwards to the start, then forwards.
    PingPong,
    /// Stop on the last frame and set `finished`.
    Once,
};

/// A run of frames along one row of a sprite sheet.
///
/// The frame index is an offset from SpriteComponent::currentCol, which is
/// where the run starts in the sheet.
struct AnimationComponent
{
    /// Offset of the current frame within the run, counted from 0.
    int currentFrame = 0;

    /// How many frames the run has.
    int totalFrames = 1;

    /// Seconds each frame is held.
    float animationTime = 0.1f;

    /// Time accumulated towards the next frame.
    float timeSinceLastFrame = 0.0f;

    AnimationLoop loop = AnimationLoop::Loop;

    /// Scales the playback rate: 2 plays twice as fast, 0 freezes it.
    float speedMultiplier = 1.0f;

    /// Seconds to hold on the first and last frame before carrying on. Zero
    /// for none. Useful for an idle that pauses at the ends of its swing.
    float holdAtEnds = 0.0f;

    /// Counts down while holding at an end.
    float holdRemaining = 0.0f;

    /// Which way a PingPong run is currently going.
    bool reversing = false;

    /// Set when a `Once` run reaches its last frame. It stops advancing until
    /// something clears this -- a death animation stays on its last frame, an
    /// attack animation is restarted by whatever triggered it.
    bool finished = false;
};

} // namespace de

#endif // DE_COMPONENTS_ANIMATION_COMPONENT_HPP

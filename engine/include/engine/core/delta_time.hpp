#ifndef DE_CORE_DELTA_TIME_HPP
#define DE_CORE_DELTA_TIME_HPP

namespace de
{
/// Frame timing, published in the registry context each frame.
struct DeltaTime
{
    /// Seconds since the previous frame, clamped. Use this for anything
    /// frame-rate dependent that is only smoothing (camera lerp, animation).
    float value = 0.0f;

    /// Length of one fixed step. Systems registered with addFixedSystem() are
    /// stepped with this and nothing else, so movement and collision advance
    /// by the same amount regardless of frame rate.
    float fixed = 1.0f / 60.0f;

    /// How far into the next fixed step the current frame sits, in [0, 1).
    /// Rendering can interpolate with this to avoid showing quantised motion.
    float alpha = 0.0f;

    /// Seconds since the loop started.
    double elapsed = 0.0;
};

} // namespace de

#endif // DE_CORE_DELTA_TIME_HPP

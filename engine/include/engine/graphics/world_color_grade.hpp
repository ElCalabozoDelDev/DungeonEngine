#ifndef DE_GRAPHICS_WORLD_COLOR_GRADE_HPP
#define DE_GRAPHICS_WORLD_COLOR_GRADE_HPP

namespace de
{
/// Optional world post-process (MonoGame `grayscaleEffect` Saturation).
///
/// `colorAmount` 1 = full colour, 0 = luminance grayscale. The game fades this
/// toward 0 while paused / game-over; UI draws after the grade and stays
/// coloured. Engine-only — no gameplay types.
struct WorldColorGrade
{
    float colorAmount = 1.0f;
};

} // namespace de

#endif // DE_GRAPHICS_WORLD_COLOR_GRADE_HPP

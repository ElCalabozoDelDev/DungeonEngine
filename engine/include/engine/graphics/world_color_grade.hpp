#ifndef DE_GRAPHICS_WORLD_COLOR_GRADE_HPP
#define DE_GRAPHICS_WORLD_COLOR_GRADE_HPP

namespace de
{
/// Optional world post-process amount.
///
/// `colorAmount` 1 = full colour, 0 = luminance grayscale (same lerp as
/// MonoGame's grayscaleEffect Saturation parameter).
struct WorldColorGrade
{
    float colorAmount = 1.0f;
};

} // namespace de

#endif // DE_GRAPHICS_WORLD_COLOR_GRADE_HPP

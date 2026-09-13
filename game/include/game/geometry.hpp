#ifndef GAME_GEOMETRY_HPP
#define GAME_GEOMETRY_HPP

#include <engine/core/vector_2d.hpp>

namespace game
{
/// Default slime/bat sprite size in logical pixels (matches tile size).
constexpr float kSegmentSize = 20.0f;

inline de::Vector2D<float> lerp(const de::Vector2D<float>& a,
                                const de::Vector2D<float>& b, float t)
{
    return a + (b - a) * t;
}

} // namespace game

#endif // GAME_GEOMETRY_HPP

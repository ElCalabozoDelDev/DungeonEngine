#ifndef GAME_GEOMETRY_HPP
#define GAME_GEOMETRY_HPP

namespace game
{
/// Default slime/bat sprite size in logical pixels (matches tile size).
/// Vector maths (lerp, reflect, Circle) lives in engine/core/math.hpp.
constexpr float kSegmentSize = 20.0f;

} // namespace game

#endif // GAME_GEOMETRY_HPP

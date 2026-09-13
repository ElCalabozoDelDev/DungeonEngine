#ifndef GAME_SYSTEMS_SNAKE_SYSTEM_HPP
#define GAME_SYSTEMS_SNAKE_SYSTEM_HPP

#include <engine/core/vector_2d.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/spatial/quadtree.hpp>
#include <engine/systems/system.hpp>
#include <game/components/snake_component.hpp>

/// The snake's rules, free of the registry so they can be tested on a bare
/// SnakeComponent. SnakeSystem applies them each fixed step; SnakeViewSystem
/// draws the result.
namespace game::snake
{
/// The direction the held keys ask for, unless it would reverse the snake
/// onto itself, in which case the current `nextDirection` stands.
de::Vector2D<float> steer(const SnakeComponent& snake,
                          const de::InputState& input,
                          const de::ActionMap& actions);

enum class StepOutcome
{
    Moved,
    HitWall,
    HitSelf
};

/// Takes one stride in `nextDirection`: a new head cell in front, the tail
/// dropped unless growth is pending. A stride that would leave `room`
/// instead freezes the head on its last floor cell and reports HitWall; one
/// whose new head overlaps the body reports HitSelf.
StepOutcome advance(SnakeComponent& snake, const de::Box<float>& room);

} // namespace game::snake

/// Steers and moves the player's snake at its stride interval, and ends the
/// run when it dies. A fixed system; it does not check for pause (see
/// PlayState).
class SnakeSystem : public de::System
{
public:
    void run(entt::registry& registry) override;
};

#endif // GAME_SYSTEMS_SNAKE_SYSTEM_HPP

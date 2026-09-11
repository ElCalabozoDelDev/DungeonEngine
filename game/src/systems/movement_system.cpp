#include <engine/components/velocity_component.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/player_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/systems/movement_system.hpp>

using namespace de;

void MovementSystem::run(entt::registry& registry)
{
    const auto& input = registry.ctx().get<InputState>();
    const auto& actions = registry.ctx().get<ActionMap>();

    auto view =
        registry.view<VelocityComponent, SpeedComponent, PlayerComponent>();

    for (auto entity : view)
    {
        auto& velocity = view.get<VelocityComponent>(entity);
        const auto& speed = view.get<SpeedComponent>(entity);

        Vector2D<float> direction(
            actions.axis(input, "move_left", "move_right"),
            actions.axis(input, "move_up", "move_down"));

        // Without this, holding two directions moved the player 1.41x faster
        // than holding one.
        direction.normalize();

        velocity.velocity = direction * speed.value;
    }
}

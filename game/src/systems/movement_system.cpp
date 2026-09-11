#include <SDL.h>
#include <engine/components/velocity_component.hpp>
#include <game/components/player_component.hpp>
#include <game/systems/movement_system.hpp>

using namespace de;

namespace
{
constexpr float PlayerSpeed = 200.0f;
}

void MovementSystem::run(entt::registry& registry)
{
    auto view = registry.view<VelocityComponent, PlayerComponent>();
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    for (auto entity : view)
    {
        auto& vel = view.get<VelocityComponent>(entity);

        vel.velocity.setX(0);
        vel.velocity.setY(0);

        if (state[SDL_SCANCODE_UP])
        {
            vel.velocity.setY(-PlayerSpeed);
        }
        if (state[SDL_SCANCODE_DOWN])
        {
            vel.velocity.setY(PlayerSpeed);
        }
        if (state[SDL_SCANCODE_LEFT])
        {
            vel.velocity.setX(-PlayerSpeed);
        }
        if (state[SDL_SCANCODE_RIGHT])
        {
            vel.velocity.setX(PlayerSpeed);
        }
    }
}

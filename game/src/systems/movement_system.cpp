#include "SDL.h"
#include "systems/movement_system.hpp"
#include "components/velocity_component.hpp"
#include "components/player_component.hpp"

void MovementSystem::handle(entt::registry& registry) {
    auto view = registry.view<VelocityComponent, PlayerComponent>();

    for (auto entity : view) {
        auto& vel = view.get<VelocityComponent>(entity);

        const Uint8* state = SDL_GetKeyboardState(nullptr);

        vel.vx = 0;
        vel.vy = 0;

        if (state[SDL_SCANCODE_UP]) {
            vel.vy = -200.0f;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            vel.vy = 200.0f;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            vel.vx = -200.0f;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            vel.vx = 200.0f;
        }
    }
}
#include "SDL.h"
#include "systems/handle_input.hpp"
#include "components/velocity.hpp"
#include "components/player_controlled.hpp"

void handle_input(entt::registry& registry) {
    auto view = registry.view<Velocity, PlayerControlled>();

    for (auto entity : view) {
        auto& vel = view.get<Velocity>(entity);

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
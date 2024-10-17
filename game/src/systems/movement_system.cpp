#include "SDL.h"
#include "systems/movement_system.hpp"
#include "components/velocity_component.hpp"
#include "components/player_component.hpp"

void MovementSystem::run(entt::registry& registry) {
	auto view = registry.view<VelocityComponent, PlayerComponent>();

	for (auto entity : view) {
		auto& vel = view.get<VelocityComponent>(entity);

		const Uint8* state = SDL_GetKeyboardState(nullptr);

		vel.velocity.setX(0);
		vel.velocity.setY(0);

		if (state[SDL_SCANCODE_UP]) {
			vel.velocity.setY(-200.0f);
		}
		if (state[SDL_SCANCODE_DOWN]) {
			vel.velocity.setY(200.0f);
		}
		if (state[SDL_SCANCODE_LEFT]) {
			vel.velocity.setX(-200.0f);
		}
		if (state[SDL_SCANCODE_RIGHT]) {
			vel.velocity.setX(200.0f);
		}
	}
}
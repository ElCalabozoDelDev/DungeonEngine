#include "systems/render_system.hpp"
#include "components/position_component.hpp"

void RenderSystem::render(SDL_Renderer* renderer, entt::registry& registry) {
    auto view = registry.view<PositionComponent>();
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (auto entity : view) {
        auto& pos = view.get<PositionComponent>(entity);
        SDL_Rect ball = {static_cast<int>(pos.x), static_cast<int>(pos.y), 20, 20};
        SDL_RenderFillRect(renderer, &ball);
    }

    SDL_RenderPresent(renderer);
}
#include "SDL.h"
#include "systems/render_entities.hpp"
#include "components/position.hpp"

void render_entities(SDL_Renderer* renderer, entt::registry& registry) {
    auto view = registry.view<Position>();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        SDL_Rect ball = {static_cast<int>(pos.x), static_cast<int>(pos.y), 20, 20};
        SDL_RenderFillRect(renderer, &ball);
    }

    SDL_RenderPresent(renderer);
}
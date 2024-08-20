#ifndef RENDER_HPP
#define RENDER_HPP

#include "SDL.h"
#include "entt/entt.hpp"

class RenderSystem {
    public:
        void render(SDL_Renderer* renderer, entt::registry& registry);
};
#endif // RENDER_HPP
#ifndef RENDER_HPP
#define RENDER_HPP

#include "SDL.h"
#include "entt.hpp"

void render_entities(SDL_Renderer* renderer, entt::registry& registry);

#endif // RENDER_HPP
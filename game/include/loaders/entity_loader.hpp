#ifndef ENTITY_LOADER_HPP
#define ENTITY_LOADER_HPP

#include <string>
#include "SDL.h"
#include "entt/entt.hpp"

class EntityLoader
{
private:
    static SDL_Texture *loadTexture(const std::string& path, SDL_Renderer* renderer);
public:
    EntityLoader() = default;
    ~EntityLoader() = default;
    static bool loadPlayerDataFromXML(entt::registry &registry, SDL_Renderer* renderer);
};

#endif // ENTITY_LOADER_HPP
#ifndef IMGUI_SYSTEM_HPP
#define IMGUI_SYSTEM_HPP

#include <SDL.h>
#include <entt/entt.hpp>

class ImGuiSystem {
public:
    ImGuiSystem() = default;
    ~ImGuiSystem() = default;

    void init(SDL_Window* window, SDL_Renderer* renderer);
    void handleEvents(SDL_Event& event);
    void newFrame();
    void render(entt::registry& registry, SDL_Renderer *renderer);
    void shutdown();
};

#endif // IMGUI_SYSTEM_HPP
#ifndef IMGUI_SYSTEM_HPP
#define IMGUI_SYSTEM_HPP

#include "core/system.hpp"
#include <SDL.h>
#include <entt/entt.hpp>

class ImGuiSystem : public System {
public:
    ImGuiSystem() = default;
    ~ImGuiSystem() = default;
    void run(entt::registry& registry) override;
    void handle(SDL_Event& event);
};

#endif // IMGUI_SYSTEM_HPP
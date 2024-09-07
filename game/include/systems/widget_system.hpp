#ifndef IMGUI_SYSTEM_HPP
#define IMGUI_SYSTEM_HPP

#include "systems/system.hpp"
#include <SDL.h>
#include <entt/entt.hpp>

class WidgetSystem : public System {
public:
    WidgetSystem() = default;
    ~WidgetSystem() = default;
    void run(entt::registry& registry) override;
    void handle(SDL_Event& event);
};

#endif // IMGUI_SYSTEM_HPP
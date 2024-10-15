#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include "systems/system.hpp"
#include "entt/entt.hpp"
#include "graphics/render.hpp"

class RenderSystem final : public System {
    public:
        void run(entt::registry& registry) override;
    private:
        void renderGraphics(entt::registry& registry);
        void renderGUI(entt::registry& registry);
};
#endif // RENDER_SYSTEM_HPP
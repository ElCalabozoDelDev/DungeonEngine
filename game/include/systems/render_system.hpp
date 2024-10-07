#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include "systems/system.hpp"
#include "entt/entt.hpp"
#include "graphics/render.hpp"

class RenderSystem final : public System {
    private:
        std::vector<std::shared_ptr<Render>> m_renders;
    public:
        void run(entt::registry& registry) override;
};
#endif // RENDER_SYSTEM_HPP
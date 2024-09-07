#ifndef RENDER_HPP
#define RENDER_HPP

#include "systems/system.hpp"
#include "entt/entt.hpp"

class RenderSystem final : public System {
    public:
        void run(entt::registry& registry) override;
};
#endif // RENDER_HPP
#ifndef DE_SYSTEMS_RENDER_SYSTEM_HPP
#define DE_SYSTEMS_RENDER_SYSTEM_HPP
#include "entt/entt.hpp"
#include <engine/systems/system.hpp>

namespace de
{
class RenderSystem final : public System
{
public:
    void run(entt::registry& registry) override;

private:
    void renderGraphics(entt::registry& registry);
    void renderGUI(entt::registry& registry);
};

} // namespace de

#endif // DE_SYSTEMS_RENDER_SYSTEM_HPP

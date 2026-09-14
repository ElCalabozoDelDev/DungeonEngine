#ifndef DE_SYSTEMS_RENDER_SYSTEM_HPP
#define DE_SYSTEMS_RENDER_SYSTEM_HPP

#include "entt/entt.hpp"
#include <engine/graphics/world_color_grade_pass.hpp>
#include <engine/systems/system.hpp>

namespace de
{
/// Draws the RenderPass entities through the active camera, then the widgets.
class RenderSystem final : public System
{
public:
    void run(entt::registry& registry) override;

    /// Destroys the textures of the world colour grade. Must run before the
    /// renderer they were created on; BasePlugin calls it at teardown.
    void releaseGpuResources() { m_gradePass.destroy(); }

private:
    void renderGraphics(entt::registry& registry);
    void renderGUI(entt::registry& registry);

    /// Owned here rather than in the registry context: only this system uses
    /// it, and the SDL plugin's teardown no longer has to know it exists.
    WorldColorGradePass m_gradePass;
};

} // namespace de

#endif // DE_SYSTEMS_RENDER_SYSTEM_HPP

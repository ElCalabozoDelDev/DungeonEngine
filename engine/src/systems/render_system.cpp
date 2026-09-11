#include <SDL_render.h>
#include <algorithm>
#include <engine/graphics/camera2d.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/widgets/widget.hpp>
#include <imgui.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

namespace de
{
void RenderSystem::run(entt::registry& registry)
{
    renderGraphics(registry);
    renderGUI(registry);
}

void RenderSystem::renderGraphics(entt::registry& registry)
{
    const auto camera = activeCamera(registry);
    if (!camera)
    {
        // No camera yet (a scene that has not loaded): nothing to draw.
        return;
    }

    // Draw order is explicit. It used to be whatever order EnTT iterated the
    // pool in, so the background landing behind the overlay was luck. Sorting
    // every frame keeps it correct when a scene adds a pass later; with a
    // handful of passes the cost is nothing.
    registry.sort<RenderPass>([](const RenderPass& lhs, const RenderPass& rhs)
                              { return lhs.order < rhs.order; });

    for (auto&& [entity, renderPass] : registry.view<RenderPass>().each())
    {
        if (renderPass.pass)
        {
            renderPass.pass->draw(registry, *camera);
        }
    }
}

void RenderSystem::renderGUI(entt::registry& registry)
{
    SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();

    for (auto&& [entity, widget] : registry.view<Widget>().each())
    {
        if (widget.widget)
        {
            widget.widget->frame_begin();
            widget.widget->frame_update(registry);
            widget.widget->frame_end();
        }
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

} // namespace de

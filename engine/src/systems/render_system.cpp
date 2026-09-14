#include <SDL_render.h>
#include <engine/graphics/camera2d.hpp>
#include <engine/graphics/logical_size.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/world_color_grade.hpp>
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
        return;
    }

    SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
    const auto* grade = registry.ctx().find<WorldColorGrade>();
    const float colorAmount = grade != nullptr ? grade->colorAmount : 1.0f;
    const bool gradeActive = colorAmount < 0.999f;

    const auto [logicalWf, logicalHf] = logicalSize(registry);
    const int logicalW = static_cast<int>(logicalWf);
    const int logicalH = static_cast<int>(logicalHf);

    auto& pass = m_gradePass;

    const bool useTarget =
        gradeActive && pass.ensure(renderer, logicalW, logicalH);
    if (useTarget)
    {
        SDL_SetRenderTarget(renderer, pass.target());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
    else if (pass.ready() && !gradeActive)
    {
        pass.destroy();
    }

    registry.sort<RenderPass>([](const RenderPass& lhs, const RenderPass& rhs)
                              { return lhs.order < rhs.order; });

    for (auto&& [entity, renderPass] : registry.view<RenderPass>().each())
    {
        if (renderPass.pass)
        {
            renderPass.pass->draw(registry, *camera);
        }
    }

    if (useTarget)
    {
        const bool ok = pass.gradeCurrentTarget(renderer, colorAmount);
        SDL_SetRenderTarget(renderer, nullptr);
        if (ok)
        {
            pass.present(renderer);
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

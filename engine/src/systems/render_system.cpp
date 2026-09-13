#include <SDL_render.h>
#include <algorithm>
#include <cstdint>
#include <engine/graphics/camera2d.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <engine/loaders/config.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/widgets/widget.hpp>
#include <imgui.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

namespace de
{
namespace
{
void applyLuminanceGrade(std::uint8_t* pixels, int count, float colorAmount)
{
    colorAmount = std::clamp(colorAmount, 0.0f, 1.0f);
    for (int i = 0; i < count; ++i)
    {
        std::uint8_t* p = pixels + i * 4;
        const float r = static_cast<float>(p[0]);
        const float g = static_cast<float>(p[1]);
        const float b = static_cast<float>(p[2]);
        // Same weights as assets/effects/grayscaleEffect.fx
        const float gray = 0.3f * r + 0.59f * g + 0.11f * b;
        p[0] = static_cast<std::uint8_t>(gray + (r - gray) * colorAmount);
        p[1] = static_cast<std::uint8_t>(gray + (g - gray) * colorAmount);
        p[2] = static_cast<std::uint8_t>(gray + (b - gray) * colorAmount);
    }
}
} // namespace

void RenderSystem::destroyGradeResources()
{
    if (m_gradeTarget != nullptr)
    {
        SDL_DestroyTexture(m_gradeTarget);
        m_gradeTarget = nullptr;
    }
    if (m_gradePresent != nullptr)
    {
        SDL_DestroyTexture(m_gradePresent);
        m_gradePresent = nullptr;
    }
    m_gradeW = 0;
    m_gradeH = 0;
    m_gradePixels.clear();
}

bool RenderSystem::ensureGradeResources(SDL_Renderer* renderer, int w, int h)
{
    if (renderer == nullptr || w <= 0 || h <= 0)
    {
        return false;
    }
    if (m_gradeTarget != nullptr && m_gradePresent != nullptr &&
        m_gradeW == w && m_gradeH == h)
    {
        return true;
    }
    destroyGradeResources();

    m_gradeTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_TARGET, w, h);
    m_gradePresent = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STREAMING, w, h);
    if (m_gradeTarget == nullptr || m_gradePresent == nullptr)
    {
        destroyGradeResources();
        return false;
    }
    SDL_SetTextureScaleMode(m_gradeTarget, SDL_ScaleModeNearest);
    SDL_SetTextureScaleMode(m_gradePresent, SDL_ScaleModeNearest);
    SDL_SetTextureBlendMode(m_gradePresent, SDL_BLENDMODE_NONE);
    m_gradeW = w;
    m_gradeH = h;
    m_gradePixels.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) *
                         4u);
    return true;
}

void RenderSystem::presentGraded(SDL_Renderer* renderer, float colorAmount)
{
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32,
                             m_gradePixels.data(), m_gradeW * 4) != 0)
    {
        return;
    }
    applyLuminanceGrade(m_gradePixels.data(), m_gradeW * m_gradeH,
                        colorAmount);
    SDL_UpdateTexture(m_gradePresent, nullptr, m_gradePixels.data(),
                      m_gradeW * 4);
}

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

    SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
    const auto* grade = registry.ctx().find<WorldColorGrade>();
    const float colorAmount = grade != nullptr ? grade->colorAmount : 1.0f;
    const bool gradeActive = colorAmount < 0.999f;

    int logicalW = 0;
    int logicalH = 0;
    if (const auto* config = registry.ctx().find<Config>(); config != nullptr)
    {
        logicalW = static_cast<int>(config->cameraWidth);
        logicalH = static_cast<int>(config->cameraHeight);
    }
    if (logicalW <= 0 || logicalH <= 0)
    {
        SDL_RenderGetLogicalSize(renderer, &logicalW, &logicalH);
    }

    const bool useTarget =
        gradeActive && ensureGradeResources(renderer, logicalW, logicalH);
    if (useTarget)
    {
        SDL_SetRenderTarget(renderer, m_gradeTarget);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
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

    if (useTarget)
    {
        presentGraded(renderer, colorAmount);
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, m_gradePresent, nullptr, nullptr);
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

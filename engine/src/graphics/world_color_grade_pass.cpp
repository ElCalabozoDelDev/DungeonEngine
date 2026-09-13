#include <algorithm>
#include <engine/graphics/world_color_grade_pass.hpp>

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

void WorldColorGradePass::destroy()
{
    if (m_target != nullptr)
    {
        SDL_DestroyTexture(m_target);
        m_target = nullptr;
    }
    if (m_present != nullptr)
    {
        SDL_DestroyTexture(m_present);
        m_present = nullptr;
    }
    m_w = 0;
    m_h = 0;
    m_pixels.clear();
}

bool WorldColorGradePass::ensure(SDL_Renderer* renderer, int w, int h)
{
    if (renderer == nullptr || w <= 0 || h <= 0)
    {
        return false;
    }
    if (m_target != nullptr && m_present != nullptr && m_w == w && m_h == h)
    {
        return true;
    }
    destroy();

    m_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                 SDL_TEXTUREACCESS_TARGET, w, h);
    m_present = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STREAMING, w, h);
    if (m_target == nullptr || m_present == nullptr)
    {
        destroy();
        return false;
    }
    SDL_SetTextureScaleMode(m_target, SDL_ScaleModeNearest);
    SDL_SetTextureScaleMode(m_present, SDL_ScaleModeNearest);
    SDL_SetTextureBlendMode(m_present, SDL_BLENDMODE_NONE);
    m_w = w;
    m_h = h;
    m_pixels.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) *
                    4u);
    return true;
}

bool WorldColorGradePass::gradeCurrentTarget(SDL_Renderer* renderer,
                                             float colorAmount)
{
    if (renderer == nullptr || !ready())
    {
        return false;
    }
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32,
                             m_pixels.data(), m_w * 4) != 0)
    {
        return false;
    }
    applyLuminanceGrade(m_pixels.data(), m_w * m_h, colorAmount);
    return SDL_UpdateTexture(m_present, nullptr, m_pixels.data(), m_w * 4) == 0;
}

void WorldColorGradePass::present(SDL_Renderer* renderer) const
{
    if (renderer == nullptr || m_present == nullptr)
    {
        return;
    }
    SDL_RenderCopy(renderer, m_present, nullptr, nullptr);
}

} // namespace de

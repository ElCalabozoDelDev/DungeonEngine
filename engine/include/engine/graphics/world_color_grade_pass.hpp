#ifndef DE_GRAPHICS_WORLD_COLOR_GRADE_PASS_HPP
#define DE_GRAPHICS_WORLD_COLOR_GRADE_PASS_HPP

#include <SDL.h>
#include <cstdint>
#include <vector>

namespace de
{
/// Owns the render-target pair used to desaturate the world (MonoGame
/// grayscaleEffect stand-in). Destroy via destroy() before the SDL_Renderer.
class WorldColorGradePass
{
public:
    WorldColorGradePass() = default;
    ~WorldColorGradePass() { destroy(); }

    WorldColorGradePass(const WorldColorGradePass&) = delete;
    WorldColorGradePass& operator=(const WorldColorGradePass&) = delete;

    WorldColorGradePass(WorldColorGradePass&& other) noexcept
        : m_target(other.m_target), m_present(other.m_present), m_w(other.m_w),
          m_h(other.m_h), m_pixels(std::move(other.m_pixels))
    {
        other.m_target = nullptr;
        other.m_present = nullptr;
        other.m_w = 0;
        other.m_h = 0;
    }

    WorldColorGradePass& operator=(WorldColorGradePass&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            m_target = other.m_target;
            m_present = other.m_present;
            m_w = other.m_w;
            m_h = other.m_h;
            m_pixels = std::move(other.m_pixels);
            other.m_target = nullptr;
            other.m_present = nullptr;
            other.m_w = 0;
            other.m_h = 0;
        }
        return *this;
    }

    void destroy();
    bool ensure(SDL_Renderer* renderer, int w, int h);
    /// Read current target, apply luminance grade, upload to present texture.
    /// Returns false if ReadPixels failed (caller must not present).
    bool gradeCurrentTarget(SDL_Renderer* renderer, float colorAmount);
    void present(SDL_Renderer* renderer) const;
    SDL_Texture* target() const { return m_target; }
    bool ready() const { return m_target != nullptr && m_present != nullptr; }

private:
    SDL_Texture* m_target = nullptr;
    SDL_Texture* m_present = nullptr;
    int m_w = 0;
    int m_h = 0;
    std::vector<std::uint8_t> m_pixels;
};

} // namespace de

#endif // DE_GRAPHICS_WORLD_COLOR_GRADE_PASS_HPP

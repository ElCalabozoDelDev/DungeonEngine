#ifndef DE_SYSTEMS_RENDER_SYSTEM_HPP
#define DE_SYSTEMS_RENDER_SYSTEM_HPP

#include "entt/entt.hpp"
#include <SDL.h>
#include <engine/systems/system.hpp>
#include <vector>

namespace de
{
class RenderSystem final : public System
{
public:
    RenderSystem() = default;
    ~RenderSystem() override { destroyGradeResources(); }

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void run(entt::registry& registry) override;

private:
    void renderGraphics(entt::registry& registry);
    void renderGUI(entt::registry& registry);

    void destroyGradeResources();
    bool ensureGradeResources(SDL_Renderer* renderer, int w, int h);
    void presentGraded(SDL_Renderer* renderer, float colorAmount);

    SDL_Texture* m_gradeTarget = nullptr;
    SDL_Texture* m_gradePresent = nullptr;
    int m_gradeW = 0;
    int m_gradeH = 0;
    std::vector<std::uint8_t> m_gradePixels;
};

} // namespace de

#endif // DE_SYSTEMS_RENDER_SYSTEM_HPP

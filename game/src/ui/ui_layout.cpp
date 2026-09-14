#include <SDL.h>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/graphics/logical_size.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>

using namespace de;

namespace game::ui
{
ImVec2 canvasSize(entt::registry& registry)
{
    const auto [w, h] = de::logicalSize(registry);
    return ImVec2(w, h);
}

float presentScale(entt::registry& registry)
{
    return de::windowToLogicalScale(registry);
}

void playUi(entt::registry& registry)
{
    if (auto* audio = registry.ctx().find<AudioManager>())
    {
        audio->playSound("ui");
    }
}

float wrapPositive(float value, float period)
{
    if (period <= 0.0f)
    {
        return 0.0f;
    }
    value = std::fmod(value, period);
    if (value < 0.0f)
    {
        value += period;
    }
    return value;
}

void drawMenuBackground(entt::registry& registry, ImDrawList* draw,
                        ImVec2 canvas, ImVec2& scroll)
{
    if (draw == nullptr)
    {
        return;
    }
    draw->AddRectFilled(ImVec2(0, 0), canvas, kClearColor);

    auto* textures = registry.ctx().find<TextureCache>();
    if (textures == nullptr)
    {
        return;
    }
    SDL_Texture* pattern = textures->get("bg-pattern");
    if (pattern == nullptr)
    {
        return;
    }
    int tw = 0;
    int th = 0;
    SDL_QueryTexture(pattern, nullptr, nullptr, &tw, &th);
    if (tw <= 0 || th <= 0)
    {
        return;
    }

    const float scale = presentScale(registry);
    const float tileW = static_cast<float>(tw) / scale;
    const float tileH = static_cast<float>(th) / scale;
    if (const auto* dt = registry.ctx().find<DeltaTime>(); dt != nullptr)
    {
        const float step = (kPatternScrollSpeed / scale) * dt->value;
        scroll.x = wrapPositive(scroll.x - step, tileW);
        scroll.y = wrapPositive(scroll.y - step, tileH);
    }
    const float ox = scroll.x;
    const float oy = scroll.y;
    // Nearest sampling and blending were set when TextureCache loaded it.
    const ImTextureID id = reinterpret_cast<ImTextureID>(pattern);
    for (float y = -oy; y < canvas.y; y += tileH)
    {
        for (float x = -ox; x < canvas.x; x += tileW)
        {
            draw->AddImage(id, ImVec2(x, y), ImVec2(x + tileW, y + tileH),
                           ImVec2(0, 0), ImVec2(1, 1), kPatternTint);
        }
    }
}

} // namespace game::ui

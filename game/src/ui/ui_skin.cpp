#include <SDL.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <engine/core/asset_paths.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <game/ui/ui_skin.hpp>
#include <string>

using namespace de;

namespace game::ui
{
namespace
{
SDL_Texture* tex(TextureCache& cache, const char* id)
{
    SDL_Texture* t = cache.get(id);
    if (t != nullptr)
    {
        SDL_SetTextureScaleMode(t, SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    }
    return t;
}

ImTextureID asId(SDL_Texture* t)
{
    return reinterpret_cast<ImTextureID>(t);
}

void loadOne(TextureCache& textures, const AssetPaths& assets, const char* id,
             const char* relative)
{
    if (textures.get(id) != nullptr)
    {
        return;
    }
    textures.load(id, assets.resolve(relative).string());
    if (SDL_Texture* t = textures.get(id); t != nullptr)
    {
        SDL_SetTextureScaleMode(t, SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    }
}
} // namespace

void ensureLoaded(entt::registry& registry)
{
    auto* textures = registry.ctx().find<TextureCache>();
    auto* assets = registry.ctx().find<AssetPaths>();
    auto* renderer = registry.ctx().find<MainRenderer>();
    if (textures == nullptr || assets == nullptr)
    {
        return;
    }
    loadOne(*textures, *assets, kPanel, "images/panel-background.png");
    loadOne(*textures, *assets, kButtonUp, "images/unfocused-button.png");
    loadOne(*textures, *assets, kButtonFocus0, "images/focused-button-1.png");
    loadOne(*textures, *assets, kButtonFocus1, "images/focused-button-2.png");
    loadOne(*textures, *assets, kSliderOff, "images/slider-off-background.png");
    loadOne(*textures, *assets, kSliderMid,
            "images/slider-middle-background.png");
    loadOne(*textures, *assets, kSliderMax, "images/slider-max-background.png");

    if (!registry.ctx().contains<BitmapFont>() && renderer != nullptr)
    {
        BitmapFont bm;
        const auto fnt = assets->resolve("fonts/04b_30.fnt").string();
        const auto page = assets->resolve("images/atlas.png").string();
        if (bm.load(renderer->get(), fnt, page))
        {
            registry.ctx().emplace<BitmapFont>(std::move(bm));
        }
    }
}

BitmapFont* font(entt::registry& registry)
{
    return registry.ctx().find<BitmapFont>();
}

int focusFrame(double elapsedSeconds)
{
    return static_cast<int>(elapsedSeconds / 0.2) % 2;
}

void drawNineSlice(ImDrawList* draw, ImTextureID texture, ImVec2 p0, ImVec2 p1,
                   float texW, float texH, float border, ImU32 tint)
{
    if (draw == nullptr || texture == nullptr || texW <= 0.0f || texH <= 0.0f)
    {
        return;
    }
    const float bw = std::min(border, (p1.x - p0.x) * 0.5f);
    const float bh = std::min(border, (p1.y - p0.y) * 0.5f);
    const float u1 = bw / texW;
    const float v1 = bh / texH;
    const float u2 = 1.0f - bw / texW;
    const float v2 = 1.0f - bh / texH;

    const float x0 = p0.x;
    const float y0 = p0.y;
    const float x1 = p0.x + bw;
    const float y1 = p0.y + bh;
    const float x2 = p1.x - bw;
    const float y2 = p1.y - bh;
    const float x3 = p1.x;
    const float y3 = p1.y;

    auto quad = [&](float xa, float ya, float xb, float yb, float ua, float va,
                    float ub, float vb)
    {
        draw->AddImage(texture, ImVec2(xa, ya), ImVec2(xb, yb), ImVec2(ua, va),
                       ImVec2(ub, vb), tint);
    };

    quad(x0, y0, x1, y1, 0, 0, u1, v1);
    quad(x1, y0, x2, y1, u1, 0, u2, v1);
    quad(x2, y0, x3, y1, u2, 0, 1, v1);
    quad(x0, y1, x1, y2, 0, v1, u1, v2);
    quad(x1, y1, x2, y2, u1, v1, u2, v2);
    quad(x2, y1, x3, y2, u2, v1, 1, v2);
    quad(x0, y2, x1, y3, 0, v2, u1, 1);
    quad(x1, y2, x2, y3, u1, v2, u2, 1);
    quad(x2, y2, x3, y3, u2, v2, 1, 1);
}

void drawPanel(entt::registry& registry, ImDrawList* draw, ImVec2 p0, ImVec2 p1,
               ImU32 tint)
{
    auto* textures = registry.ctx().find<TextureCache>();
    if (textures == nullptr || draw == nullptr)
    {
        return;
    }
    SDL_Texture* panel = tex(*textures, kPanel);
    if (panel == nullptr)
    {
        return;
    }
    int tw = 0;
    int th = 0;
    SDL_QueryTexture(panel, nullptr, nullptr, &tw, &th);
    // panel-background.png centre is alpha 175 — keep it; no opaque veil.
    drawNineSlice(draw, asId(panel), p0, p1, static_cast<float>(tw),
                  static_cast<float>(th), kPanelBorder, tint);
}

ImVec2 buttonSizeForLabel(entt::registry& registry, std::string_view label)
{
    if (BitmapFont* bm = font(registry); bm != nullptr && bm->ok())
    {
        const ImVec2 text = bm->measure(label, kFontButton);
        return ImVec2(std::max(kButtonMinW, text.x + 16.0f), kButtonH);
    }
    return ImVec2(kButtonMinW, kButtonH);
}

void drawButton(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                ImVec2 size, bool focused, double elapsedSeconds)
{
    auto* textures = registry.ctx().find<TextureCache>();
    if (textures == nullptr || draw == nullptr)
    {
        return;
    }
    const char* id = kButtonUp;
    if (focused)
    {
        id = focusFrame(elapsedSeconds) == 0 ? kButtonFocus0 : kButtonFocus1;
    }
    SDL_Texture* button = tex(*textures, id);
    if (button == nullptr)
    {
        return;
    }
    int tw = 0;
    int th = 0;
    SDL_QueryTexture(button, nullptr, nullptr, &tw, &th);
    drawNineSlice(draw, asId(button), p0, ImVec2(p0.x + size.x, p0.y + size.y),
                  static_cast<float>(tw), static_cast<float>(th), 6.0f);
}

void drawOptionsSlider(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                       std::string_view label, float value01, bool focused)
{
    auto* textures = registry.ctx().find<TextureCache>();
    if (textures == nullptr || draw == nullptr)
    {
        return;
    }
    value01 = std::clamp(value01, 0.0f, 1.0f);
    const ImVec2 p1(p0.x + kSliderPanelW, p0.y + kSliderPanelH);
    const ImU32 tint =
        focused ? IM_COL32_WHITE : IM_COL32(160, 160, 160, 255);
    drawPanel(registry, draw, p0, p1, tint);

    const ImU32 textColor =
        focused ? IM_COL32_WHITE : IM_COL32(160, 160, 160, 255);
    drawText(registry, draw, ImVec2(p0.x + 10.0f, p0.y + 10.0f), label,
             kFontPanelTitle, textColor);

    const ImVec2 inner0(p0.x + 10.0f, p0.y + 33.0f);
    const float innerW = 241.0f;
    const float innerH = 13.0f;
    constexpr float kOffW = 28.0f;
    constexpr float kMaxW = 36.0f;
    constexpr float kMidW = 179.0f;

    SDL_Texture* off = tex(*textures, kSliderOff);
    SDL_Texture* mid = tex(*textures, kSliderMid);
    SDL_Texture* max = tex(*textures, kSliderMax);
    if (off == nullptr || mid == nullptr || max == nullptr)
    {
        return;
    }

    const ImVec2 off1(inner0.x + kOffW, inner0.y + innerH);
    drawNineSlice(draw, asId(off), inner0, off1, 11.0f, 10.0f, 3.0f, tint);
    drawCenteredText(registry, draw, inner0, off1, "OFF", kFontButton,
                     kButtonTextColor);

    const ImVec2 mid0(inner0.x + 27.0f, inner0.y);
    const ImVec2 mid1(mid0.x + kMidW, mid0.y + innerH);
    drawNineSlice(draw, asId(mid), mid0, mid1, 3.0f, 3.0f, 1.0f, tint);

    const ImVec2 max0(inner0.x + innerW - kMaxW, inner0.y);
    const ImVec2 max1(max0.x + kMaxW, max0.y + innerH);
    drawNineSlice(draw, asId(max), max0, max1, 11.0f, 10.0f, 3.0f, tint);
    drawCenteredText(registry, draw, max0, max1, "MAX", kFontButton,
                     kButtonTextColor);

    // Gum ColoredRectangleRuntime uses opaque White/Gray on the track only.
    const float fillW = (kMidW - 4.0f) * value01;
    if (fillW > 0.0f)
    {
        const ImU32 fill =
            focused ? IM_COL32_WHITE : IM_COL32(160, 160, 160, 255);
        draw->AddRectFilled(ImVec2(mid0.x + 2.0f, mid0.y + 2.0f),
                            ImVec2(mid0.x + 2.0f + fillW, mid1.y - 2.0f), fill);
    }
}

void drawCenteredText(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                      ImVec2 p1, std::string_view text, float fontScale,
                      ImU32 color)
{
    BitmapFont* bm = font(registry);
    if (bm == nullptr || !bm->ok() || draw == nullptr)
    {
        return;
    }
    const ImVec2 size = bm->measure(text, fontScale);
    const ImVec2 pos(std::floor((p0.x + p1.x - size.x) * 0.5f),
                     std::floor((p0.y + p1.y - size.y) * 0.5f));
    bm->draw(draw, pos, text, fontScale, color);
}

void drawText(entt::registry& registry, ImDrawList* draw, ImVec2 pos,
              std::string_view text, float fontScale, ImU32 color)
{
    BitmapFont* bm = font(registry);
    if (bm == nullptr || !bm->ok() || draw == nullptr)
    {
        return;
    }
    bm->draw(draw, pos, text, fontScale, color);
}

bool hitButton(const char* id, ImVec2 p0, ImVec2 size)
{
    ImGui::SetNextWindowPos(p0);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(id, nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
    const bool clicked = ImGui::InvisibleButton(id, size);
    ImGui::End();
    ImGui::PopStyleVar();
    return clicked;
}

} // namespace game::ui

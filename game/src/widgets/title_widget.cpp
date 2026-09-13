#include <SDL.h>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/options_scene.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/title_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

namespace
{
ImVec2 logicalSize(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>(); config != nullptr)
    {
        return ImVec2(config->cameraWidth, config->cameraHeight);
    }
    return ImVec2(game::ui::kCanvasW, game::ui::kCanvasH);
}

float windowToLogicalScale(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>();
        config != nullptr && config->cameraWidth > 0.0f)
    {
        return static_cast<float>(config->screenWidth) / config->cameraWidth;
    }
    return 4.0f;
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

constexpr float kWindowScrollSpeed = 50.0f;
} // namespace

void TitleWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    game::ui::ensureLoaded(registry);

    const ImVec2 screen = logicalSize(registry);
    const float scale = windowToLogicalScale(registry);
    auto [focus, setFocus] = h.use_state(0);
    auto [scroll, setScroll] = h.use_state(ImVec2(0.0f, 0.0f));
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;

    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    bg->AddRectFilled(ImVec2(0, 0), screen, IM_COL32(32, 40, 78, 255));

    if (auto* textures = registry.ctx().find<TextureCache>();
        textures != nullptr)
    {
        if (SDL_Texture* pattern = textures->get("bg-pattern");
            pattern != nullptr)
        {
            int tw = 0;
            int th = 0;
            SDL_QueryTexture(pattern, nullptr, nullptr, &tw, &th);
            if (tw > 0 && th > 0)
            {
                const float tileW = static_cast<float>(tw) / scale;
                const float tileH = static_cast<float>(th) / scale;
                float ox = scroll.x;
                float oy = scroll.y;
                if (const auto* dt = registry.ctx().find<DeltaTime>();
                    dt != nullptr)
                {
                    const float step =
                        (kWindowScrollSpeed / scale) * dt->value;
                    ox = wrapPositive(ox - step, tileW);
                    oy = wrapPositive(oy - step, tileH);
                    setScroll(ImVec2(ox, oy));
                }
                SDL_SetTextureScaleMode(pattern, SDL_ScaleModeNearest);
                SDL_SetTextureBlendMode(pattern, SDL_BLENDMODE_BLEND);
                const ImTextureID id = reinterpret_cast<ImTextureID>(pattern);
                const ImU32 tint = IM_COL32(255, 255, 255, 128);
                for (float y = -oy; y < screen.y; y += tileH)
                {
                    for (float x = -ox; x < screen.x; x += tileW)
                    {
                        bg->AddImage(id, ImVec2(x, y),
                                     ImVec2(x + tileW, y + tileH),
                                     ImVec2(0, 0), ImVec2(1, 1), tint);
                    }
                }
            }
        }
    }

    // Title: BMFont at native logical size (scale 1). Each atlas texel maps
    // 1:1 in logical space, then SDL nearest-presents ×4 — chunky pixels, no
    // TTF antialias fringe. Positions / shadow match TitleScene screen space.
    if (game::ui::BitmapFont* bm = game::ui::font(registry);
        bm != nullptr && bm->ok())
    {
        const ImVec2 dungeonCenter(640.0f / scale, 100.0f / scale);
        const ImVec2 slimeCenter(757.0f / scale, 207.0f / scale);
        const ImVec2 shadow(10.0f / scale, 10.0f / scale);
        auto drawTitle = [&](const char* text, ImVec2 center)
        {
            const ImVec2 size = bm->measure(text, game::ui::kFontTitle);
            const ImVec2 topLeft(std::floor(center.x - size.x * 0.5f),
                                 std::floor(center.y - size.y * 0.5f));
            bm->draw(bg,
                     ImVec2(topLeft.x + shadow.x, topLeft.y + shadow.y), text,
                     game::ui::kFontTitle, game::ui::kTitleShadow);
            bm->draw(bg, topLeft, text, game::ui::kFontTitle, IM_COL32_WHITE);
        };
        drawTitle("Dungeon", dungeonCenter);
        drawTitle("Slime", slimeCenter);
    }

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input != nullptr && actions != nullptr)
    {
        if (actions->wasPressedRaw(*input, "move_left") ||
            actions->wasPressedRaw(*input, "move_up"))
        {
            setFocus(0);
            playUi(registry);
        }
        if (actions->wasPressedRaw(*input, "move_right") ||
            actions->wasPressedRaw(*input, "move_down"))
        {
            setFocus(1);
            playUi(registry);
        }
        if (actions->wasPressedRaw(*input, "pause"))
        {
            registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
        }
    }

    const ImVec2 startSize =
        game::ui::buttonSizeForLabel(registry, "Start");
    const ImVec2 optionsSize =
        game::ui::buttonSizeForLabel(registry, "Options");
    const ImVec2 start0(50.0f, screen.y - 12.0f - startSize.y);
    const ImVec2 options0(screen.x - 50.0f - optionsSize.x,
                          screen.y - 12.0f - optionsSize.y);

    game::ui::drawButton(registry, bg, start0, startSize, focus == 0, elapsed);
    game::ui::drawCenteredText(registry, bg, start0,
                               ImVec2(start0.x + startSize.x,
                                      start0.y + startSize.y),
                               "Start", game::ui::kFontButton,
                               game::ui::kButtonTextColor);
    game::ui::drawButton(registry, bg, options0, optionsSize, focus == 1,
                         elapsed);
    game::ui::drawCenteredText(registry, bg, options0,
                               ImVec2(options0.x + optionsSize.x,
                                      options0.y + optionsSize.y),
                               "Options", game::ui::kFontButton,
                               game::ui::kButtonTextColor);

    const bool confirm = input != nullptr && actions != nullptr &&
                         actions->wasPressedRaw(*input, "confirm");
    if (game::ui::hitButton("title-start", start0, startSize) ||
        (confirm && focus == 0))
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<InGameScene>());
    }
    if (game::ui::hitButton("title-options", options0, optionsSize) ||
        (confirm && focus == 1))
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<OptionsScene>());
    }
}

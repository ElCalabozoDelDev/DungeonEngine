#include <SDL.h>
#include <cmath>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/options_widget.hpp>
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

void applyVolume(entt::registry& registry, const AudioSettings& settings,
                 int focus)
{
    if (auto* audio = registry.ctx().find<AudioManager>())
    {
        if (focus == 0)
        {
            audio->setMusicVolume(settings.musicPercent);
        }
        else if (focus == 1)
        {
            audio->setSfxVolume(settings.sfxPercent);
        }
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
} // namespace

void OptionsWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    game::ui::ensureLoaded(registry);

    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }
    auto& settings = registry.ctx().get<AudioSettings>();
    const ImVec2 screen = logicalSize(registry);
    const float scale = windowToLogicalScale(registry);
    auto [focus, setFocus] = h.use_state(0);
    auto [scroll, setScroll] = h.use_state(ImVec2(0.0f, 0.0f));
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0, 0), screen, IM_COL32(32, 40, 78, 255));

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
                    const float step = (50.0f / scale) * dt->value;
                    ox = wrapPositive(ox - step, tileW);
                    oy = wrapPositive(oy - step, tileH);
                    setScroll(ImVec2(ox, oy));
                }
                SDL_SetTextureScaleMode(pattern, SDL_ScaleModeNearest);
                const ImTextureID id = reinterpret_cast<ImTextureID>(pattern);
                const ImU32 tint = IM_COL32(255, 255, 255, 128);
                for (float y = -oy; y < screen.y; y += tileH)
                {
                    for (float x = -ox; x < screen.x; x += tileW)
                    {
                        draw->AddImage(id, ImVec2(x, y),
                                       ImVec2(x + tileW, y + tileH),
                                       ImVec2(0, 0), ImVec2(1, 1), tint);
                    }
                }
            }
        }
    }

    ImDrawList* fg = ImGui::GetForegroundDrawList();

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input != nullptr && actions != nullptr)
    {
        if (actions->wasPressedRaw(*input, "move_up"))
        {
            setFocus((focus + 2) % 3);
            playUi(registry);
        }
        if (actions->wasPressedRaw(*input, "move_down"))
        {
            setFocus((focus + 1) % 3);
            playUi(registry);
        }
        if (focus == 0 || focus == 1)
        {
            int* value =
                focus == 0 ? &settings.musicPercent : &settings.sfxPercent;
            bool changed = false;
            if (actions->wasPressedRaw(*input, "move_left"))
            {
                *value = (*value >= 10) ? *value - 10 : 0;
                changed = true;
            }
            if (actions->wasPressedRaw(*input, "move_right"))
            {
                *value = (*value <= 90) ? *value + 10 : 100;
                changed = true;
            }
            if (changed)
            {
                playUi(registry);
                applyVolume(registry, settings, focus);
            }
        }
    }

    game::ui::drawText(registry, fg, ImVec2(10.0f, 10.0f), "OPTIONS",
                       game::ui::kFontPanelTitle, IM_COL32_WHITE);

    const float sliderX = (screen.x - game::ui::kSliderPanelW) * 0.5f;
    game::ui::drawOptionsSlider(registry, fg, ImVec2(sliderX, 30.0f), "MUSIC",
                                settings.musicPercent / 100.0f, focus == 0);
    game::ui::drawOptionsSlider(registry, fg, ImVec2(sliderX, 93.0f), "SFX",
                                settings.sfxPercent / 100.0f, focus == 1);

    const ImVec2 backSize = game::ui::buttonSizeForLabel(registry, "BACK");
    const ImVec2 back0(screen.x - 28.0f - backSize.x,
                       screen.y - 10.0f - backSize.y);
    game::ui::drawButton(registry, fg, back0, backSize, focus == 2, elapsed);
    game::ui::drawCenteredText(
        registry, fg, back0,
        ImVec2(back0.x + backSize.x, back0.y + backSize.y), "BACK",
        game::ui::kFontButton, game::ui::kButtonTextColor);

    const bool backHit = game::ui::hitButton("options-back", back0, backSize);
    const bool backPressed =
        backHit ||
        (input != nullptr && actions != nullptr &&
         (actions->wasPressedRaw(*input, "pause") ||
          (actions->wasPressedRaw(*input, "confirm") && focus == 2)));

    if (backPressed)
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }
}

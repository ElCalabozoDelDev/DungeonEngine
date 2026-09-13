#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/options_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

namespace
{
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
} // namespace

void OptionsWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    game::ui::ensureLoaded(registry);

    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }
    auto& settings = registry.ctx().get<AudioSettings>();
    const ImVec2 canvas = game::ui::canvasSize(registry);
    auto [focus, setFocus] = h.use_state(0);
    auto [scroll, setScroll] = h.use_state(ImVec2(0.0f, 0.0f));
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0, 0), canvas, game::ui::kClearColor);
    game::ui::drawScrollingPattern(registry, draw, canvas, scroll, setScroll);

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input != nullptr && actions != nullptr)
    {
        if (actions->wasPressedRaw(*input, "move_up"))
        {
            setFocus((focus + 2) % 3);
            game::ui::playUi(registry);
        }
        if (actions->wasPressedRaw(*input, "move_down"))
        {
            setFocus((focus + 1) % 3);
            game::ui::playUi(registry);
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
                game::ui::playUi(registry);
                applyVolume(registry, settings, focus);
            }
        }
    }

    game::ui::drawText(
        registry, draw,
        ImVec2(game::ui::kOptionsLabelX, game::ui::kOptionsLabelY), "OPTIONS",
        game::ui::kFontPanelTitle, IM_COL32_WHITE);

    const float sliderX = (canvas.x - game::ui::kSliderPanelW) * 0.5f;
    game::ui::drawOptionsSlider(
        registry, draw, ImVec2(sliderX, game::ui::kOptionsMusicY), "MUSIC",
        settings.musicPercent / 100.0f, focus == 0);
    game::ui::drawOptionsSlider(registry, draw,
                                ImVec2(sliderX, game::ui::kOptionsSfxY), "SFX",
                                settings.sfxPercent / 100.0f, focus == 1);

    const ImVec2 backSize = game::ui::buttonSizeForLabel(registry, "BACK");
    const ImVec2 back0(canvas.x - game::ui::kOptionsBackMarginX - backSize.x,
                       canvas.y - game::ui::kOptionsBackMarginY - backSize.y);
    game::ui::drawButton(registry, draw, back0, backSize, focus == 2, elapsed);
    game::ui::drawCenteredText(
        registry, draw, back0,
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
        game::ui::playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }
}

#include <engine/audio/audio_manager.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/menu.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/options_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;
using namespace game::ui;

namespace
{
enum Item
{
    Music,
    Sfx,
    Back
};

/// Left/right on a focused slider: a step of 10, within 0..100. Returns true
/// when a key was pressed.
bool adjustVolume(entt::registry& registry, int& percent)
{
    bool changed = false;
    if (pressed(registry, "move_left"))
    {
        percent = (percent >= 10) ? percent - 10 : 0;
        changed = true;
    }
    if (pressed(registry, "move_right"))
    {
        percent = (percent <= 90) ? percent + 10 : 100;
        changed = true;
    }
    return changed;
}

} // namespace

void OptionsWidget::render(entt::registry& registry)
{
    auto& settings = registry.ctx().get<AudioSettings>();
    const ImVec2 canvas = canvasSize(registry);
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    drawMenuBackground(registry, draw, canvas, m_scroll);

    m_nav.update(registry, {"move_up"}, {"move_down"});

    if (m_nav.focused(Music) || m_nav.focused(Sfx))
    {
        const bool music = m_nav.focused(Music);
        if (adjustVolume(registry,
                         music ? settings.musicPercent : settings.sfxPercent))
        {
            playUi(registry);
            if (auto* audio = registry.ctx().find<AudioManager>())
            {
                if (music)
                {
                    audio->setMusicVolume(settings.musicPercent);
                }
                else
                {
                    audio->setSfxVolume(settings.sfxPercent);
                }
            }
        }
    }

    drawText(registry, draw, ImVec2(kOptionsLabelX, kOptionsLabelY), "OPTIONS",
             kFontPanelTitle, IM_COL32_WHITE);

    const float sliderX = (canvas.x - kSliderPanelW) * 0.5f;
    drawOptionsSlider(registry, draw, ImVec2(sliderX, kOptionsMusicY), "MUSIC",
                      settings.musicPercent / 100.0f, m_nav.focused(Music));
    drawOptionsSlider(registry, draw, ImVec2(sliderX, kOptionsSfxY), "SFX",
                      settings.sfxPercent / 100.0f, m_nav.focused(Sfx));

    const ImVec2 backSize = buttonSizeForLabel(registry, "BACK");
    const bool back =
        menuButton(registry, draw, "options-back", "BACK",
                   ImVec2(canvas.x - kOptionsBackMarginX - backSize.x,
                          canvas.y - kOptionsBackMarginY - backSize.y),
                   m_nav.focused(Back), pressed(registry, "confirm")) ||
        pressed(registry, "pause");

    if (back)
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }
}

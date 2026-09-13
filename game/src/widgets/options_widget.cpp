#include <engine/audio/audio_manager.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
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
    return ImVec2(320.0f, 180.0f);
}

void playUi(entt::registry& registry)
{
    if (auto* audio = registry.ctx().find<AudioManager>())
    {
        audio->playSound("ui");
    }
}

bool menuButton(const char* label, bool selected, const ImVec2& size)
{
    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.55f, 0.65f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.65f, 0.75f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.45f, 0.55f, 0.85f, 1.0f));
    }
    const bool clicked = ImGui::Button(label, size);
    if (selected)
    {
        ImGui::PopStyleColor(3);
    }
    return clicked;
}
} // namespace

void OptionsWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }
    auto& settings = registry.ctx().get<AudioSettings>();
    const ImVec2 screen = logicalSize(registry);
    // 0 = music, 1 = sfx, 2 = back
    auto [focus, setFocus] = h.use_state(0);

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
                *value = (*value >= 5) ? *value - 5 : 0;
                changed = true;
            }
            if (actions->wasPressedRaw(*input, "move_right"))
            {
                *value = (*value <= 95) ? *value + 5 : 100;
                changed = true;
            }
            if (changed)
            {
                playUi(registry);
                if (auto* audio = registry.ctx().find<AudioManager>())
                {
                    if (focus == 0)
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
    }

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(240.0f, 130.0f), ImGuiCond_Always);
    ImGui::Begin("options", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::SetWindowFontScale(0.7f);
    ImGui::TextUnformatted("OPTIONS");
    ImGui::SetWindowFontScale(0.55f);
    ImGui::Separator();

    if (focus == 0)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.35f, 0.45f, 0.75f, 1.0f));
    }
    if (ImGui::SliderInt("MUSIC", &settings.musicPercent, 0, 100))
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->setMusicVolume(settings.musicPercent);
        }
    }
    if (focus == 0)
    {
        ImGui::PopStyleColor();
    }

    if (focus == 1)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.35f, 0.45f, 0.75f, 1.0f));
    }
    if (ImGui::SliderInt("SFX", &settings.sfxPercent, 0, 100))
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->setSfxVolume(settings.sfxPercent);
        }
    }
    if (focus == 1)
    {
        ImGui::PopStyleColor();
    }

    const bool backPressed =
        input != nullptr && actions != nullptr &&
        (actions->wasPressedRaw(*input, "pause") ||
         (actions->wasPressedRaw(*input, "confirm") && focus == 2));

    if (menuButton("BACK", focus == 2, ImVec2(80, 0)) || backPressed)
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

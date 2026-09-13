#include <engine/audio/audio_manager.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/options_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

void OptionsWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }
    auto& settings = registry.ctx().get<AudioSettings>();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::Begin("options", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::TextUnformatted("OPTIONS");
    ImGui::Separator();

    if (ImGui::SliderInt("MUSIC", &settings.musicPercent, 0, 100))
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->setMusicVolume(settings.musicPercent);
        }
    }
    if (ImGui::SliderInt("SFX", &settings.sfxPercent, 0, 100))
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->setSfxVolume(settings.sfxPercent);
        }
    }

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const bool backPressed = input != nullptr && actions != nullptr &&
                             actions->wasPressed(*input, "pause");

    if (ImGui::Button("BACK", ImVec2(160, 0)) || backPressed)
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->playSound("ui");
        }
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }

    ImGui::End();
}

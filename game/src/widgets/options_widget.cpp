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
} // namespace

void OptionsWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }
    auto& settings = registry.ctx().get<AudioSettings>();
    const ImVec2 screen = logicalSize(registry);

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_Always);
    ImGui::Begin("options", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::SetWindowFontScale(0.7f);
    ImGui::TextUnformatted("OPTIONS");
    ImGui::SetWindowFontScale(0.55f);
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

    if (ImGui::Button("BACK", ImVec2(80, 0)) || backPressed)
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->playSound("ui");
        }
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<TitleScene>());
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

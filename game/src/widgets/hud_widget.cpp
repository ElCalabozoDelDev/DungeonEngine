#include <engine/audio/audio_manager.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/hud_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

namespace
{
/// ImGui draws through the SDL renderer, which is in logical space
/// (cameraWidth x cameraHeight). GetMainViewport() reports the window size
/// (e.g. 1280x720), so centering there puts the panel off-canvas.
ImVec2 logicalSize(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>(); config != nullptr)
    {
        return ImVec2(config->cameraWidth, config->cameraHeight);
    }
    return ImVec2(320.0f, 180.0f);
}
} // namespace

void HudWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const auto* state = registry.ctx().find<GameState>();
    const ImVec2 screen = logicalSize(registry);

    ImGui::SetNextWindowPos(ImVec2(4, 4), ImGuiCond_Always);
    ImGui::Begin("HUD", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    if (state != nullptr)
    {
        ImGui::SetWindowFontScale(0.6f);
        ImGui::Text("SCORE: %06d", state->score);
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();

    if (state == nullptr)
    {
        return;
    }

    if (state->playState == PlayState::Paused)
    {
        const ImVec2 panel(180.0f, 70.0f);
        ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(panel, ImGuiCond_Always);
        ImGui::Begin("Paused", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar);
        ImGui::SetWindowFontScale(0.7f);
        ImGui::TextUnformatted("PAUSED");
        ImGui::SetWindowFontScale(0.55f);
        ImGui::TextUnformatted("Esc resumes.");
        if (ImGui::Button("QUIT", ImVec2(70, 0)))
        {
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<TitleScene>());
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
        return;
    }

    if (state->playState == PlayState::GameOver)
    {
        const ImVec2 panel(220.0f, 90.0f);
        ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(panel, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg,
                              ImVec4(0.15f, 0.18f, 0.35f, 0.92f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(0.75f, 0.85f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::Begin("GameOver", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar);
        ImGui::SetWindowFontScale(0.85f);
        const float titleWidth = ImGui::CalcTextSize("GAME OVER").x;
        ImGui::SetCursorPosX((panel.x - titleWidth) * 0.5f);
        ImGui::TextUnformatted("GAME OVER");
        ImGui::SetWindowFontScale(0.55f);
        ImGui::Dummy(ImVec2(0, 6));

        const float buttonWidth = 80.0f;
        const float gap = 12.0f;
        const float rowWidth = buttonWidth * 2.0f + gap;
        ImGui::SetCursorPosX((panel.x - rowWidth) * 0.5f);
        if (ImGui::Button("RETRY", ImVec2(buttonWidth, 0)))
        {
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<InGameScene>());
        }
        ImGui::SameLine(0.0f, gap);
        if (ImGui::Button("QUIT", ImVec2(buttonWidth, 0)))
        {
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<TitleScene>());
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }
}

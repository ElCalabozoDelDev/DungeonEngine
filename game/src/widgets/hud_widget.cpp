#include <engine/audio/audio_manager.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
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

/// Draw a menu button; returns true when activated by click or keyboard.
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

void HudWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const auto* state = registry.ctx().find<GameState>();
    const ImVec2 screen = logicalSize(registry);
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();

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
        // 0 = resume (Esc), 1 = quit
        auto [focus, setFocus] = h.use_state(0);
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
        }

        const ImVec2 panel(200.0f, 80.0f);
        ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(panel, ImGuiCond_Always);
        ImGui::Begin("Paused", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar);
        ImGui::SetWindowFontScale(0.7f);
        ImGui::TextUnformatted("PAUSED");
        ImGui::SetWindowFontScale(0.5f);
        ImGui::TextUnformatted("Arrows + Enter");
        ImGui::Dummy(ImVec2(0, 4));

        const float buttonWidth = 80.0f;
        const float gap = 10.0f;
        ImGui::SetCursorPosX((panel.x - (buttonWidth * 2.0f + gap)) * 0.5f);

        const bool confirm = input != nullptr && actions != nullptr &&
                             actions->wasPressedRaw(*input, "confirm");

        if (menuButton("RESUME", focus == 0, ImVec2(buttonWidth, 0)) ||
            (confirm && focus == 0))
        {
            playUi(registry);
            auto* mutableState = registry.ctx().find<GameState>();
            if (mutableState != nullptr)
            {
                mutableState->playState = PlayState::Playing;
            }
            registry.ctx().get<Paused>().value = false;
        }
        ImGui::SameLine(0.0f, gap);
        if (menuButton("QUIT", focus == 1, ImVec2(buttonWidth, 0)) ||
            (confirm && focus == 1))
        {
            playUi(registry);
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
        // 0 = retry, 1 = quit
        auto [focus, setFocus] = h.use_state(0);
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
        }

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

        const bool confirm = input != nullptr && actions != nullptr &&
                             actions->wasPressedRaw(*input, "confirm");

        if (menuButton("RETRY", focus == 0, ImVec2(buttonWidth, 0)) ||
            (confirm && focus == 0))
        {
            playUi(registry);
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<InGameScene>());
        }
        ImGui::SameLine(0.0f, gap);
        if (menuButton("QUIT", focus == 1, ImVec2(buttonWidth, 0)) ||
            (confirm && focus == 1))
        {
            playUi(registry);
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

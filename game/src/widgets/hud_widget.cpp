#include <cstdio>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/paused.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/ui_skin.hpp>
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
    return ImVec2(game::ui::kCanvasW, game::ui::kCanvasH);
}

void playUi(entt::registry& registry)
{
    if (auto* audio = registry.ctx().find<AudioManager>())
    {
        audio->playSound("ui");
    }
}

void handleSideFocus(entt::registry& registry, const InputState* input,
                     const ActionMap* actions,
                     const de::gui::setter_fn_type<int>& setFocus)
{
    if (input == nullptr || actions == nullptr)
    {
        return;
    }
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

/// GameSceneUI pause / game-over panel: 264×70, title at (10,10),
/// left button BottomLeft (9,-9), right BottomRight (-9,-9).
void drawPauseLikePanel(entt::registry& registry, ImDrawList* draw,
                        const ImVec2& screen, const char* title,
                        const char* leftLabel, const char* rightLabel,
                        int focus, double elapsed, bool confirm,
                        bool& leftActivated, bool& rightActivated)
{
    const float panelW = game::ui::kPausePanelW;
    const float panelH = game::ui::kPausePanelH;
    const ImVec2 panel0((screen.x - panelW) * 0.5f, (screen.y - panelH) * 0.5f);
    const ImVec2 panel1(panel0.x + panelW, panel0.y + panelH);
    game::ui::drawPanel(registry, draw, panel0, panel1);

    game::ui::drawText(registry, draw, ImVec2(panel0.x + 10.0f, panel0.y + 10.0f),
                       title, game::ui::kFontPanelTitle, IM_COL32_WHITE);

    const ImVec2 leftSize = game::ui::buttonSizeForLabel(registry, leftLabel);
    const ImVec2 rightSize = game::ui::buttonSizeForLabel(registry, rightLabel);
    const ImVec2 left0(panel0.x + 9.0f, panel1.y - 9.0f - leftSize.y);
    const ImVec2 right0(panel1.x - 9.0f - rightSize.x,
                        panel1.y - 9.0f - rightSize.y);

    game::ui::drawButton(registry, draw, left0, leftSize, focus == 0, elapsed);
    game::ui::drawCenteredText(
        registry, draw, left0,
        ImVec2(left0.x + leftSize.x, left0.y + leftSize.y), leftLabel,
        game::ui::kFontButton, game::ui::kButtonTextColor);
    game::ui::drawButton(registry, draw, right0, rightSize, focus == 1,
                         elapsed);
    game::ui::drawCenteredText(
        registry, draw, right0,
        ImVec2(right0.x + rightSize.x, right0.y + rightSize.y), rightLabel,
        game::ui::kFontButton, game::ui::kButtonTextColor);

    leftActivated =
        game::ui::hitButton(leftLabel, left0, leftSize) || (confirm && focus == 0);
    rightActivated = game::ui::hitButton(rightLabel, right0, rightSize) ||
                     (confirm && focus == 1);
}
} // namespace

void HudWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    game::ui::ensureLoaded(registry);

    const auto* state = registry.ctx().find<GameState>();
    const ImVec2 screen = logicalSize(registry);
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    if (state != nullptr)
    {
        // GameSceneUI CreateScoreText: TopLeft X=20 Y=5, FontScale 0.25.
        char scoreLabel[32];
        std::snprintf(scoreLabel, sizeof(scoreLabel), "SCORE: %06d",
                      state->score);
        game::ui::drawText(registry, draw, ImVec2(20.0f, 5.0f), scoreLabel,
                           game::ui::kFontScore, IM_COL32_WHITE);
    }

    if (state == nullptr)
    {
        return;
    }

    const bool confirm = input != nullptr && actions != nullptr &&
                         actions->wasPressedRaw(*input, "confirm");

    if (state->playState == PlayState::Paused)
    {
        auto [focus, setFocus] = h.use_state(0);
        handleSideFocus(registry, input, actions, setFocus);

        bool left = false;
        bool right = false;
        drawPauseLikePanel(registry, draw, screen, "PAUSED", "RESUME", "QUIT",
                           focus, elapsed, confirm, left, right);
        if (left)
        {
            playUi(registry);
            if (auto* mutableState = registry.ctx().find<GameState>();
                mutableState != nullptr)
            {
                mutableState->playState = PlayState::Playing;
            }
            registry.ctx().get<Paused>().value = false;
        }
        else if (right)
        {
            playUi(registry);
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<TitleScene>());
        }
        return;
    }

    if (state->playState == PlayState::GameOver)
    {
        auto [focus, setFocus] = h.use_state(0);
        handleSideFocus(registry, input, actions, setFocus);

        bool left = false;
        bool right = false;
        drawPauseLikePanel(registry, draw, screen, "GAME OVER", "RETRY", "QUIT",
                           focus, elapsed, confirm, left, right);
        if (left)
        {
            playUi(registry);
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<InGameScene>());
        }
        else if (right)
        {
            playUi(registry);
            registry.ctx().get<Paused>().value = false;
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<TitleScene>());
        }
    }
}

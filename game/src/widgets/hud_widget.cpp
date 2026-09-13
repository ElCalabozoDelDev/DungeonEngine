#include <cstdio>
#include <engine/core/delta_time.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/play_state.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/hud_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

namespace
{
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
        game::ui::playUi(registry);
    }
    if (actions->wasPressedRaw(*input, "move_right") ||
        actions->wasPressedRaw(*input, "move_down"))
    {
        setFocus(1);
        game::ui::playUi(registry);
    }
}

void drawPauseLikePanel(entt::registry& registry, ImDrawList* draw,
                        const ImVec2& canvas, const char* title,
                        const char* leftLabel, const char* rightLabel,
                        int focus, double elapsed, bool confirm,
                        bool& leftActivated, bool& rightActivated)
{
    const float panelW = game::ui::kPausePanelW;
    const float panelH = game::ui::kPausePanelH;
    const ImVec2 panel0((canvas.x - panelW) * 0.5f, (canvas.y - panelH) * 0.5f);
    const ImVec2 panel1(panel0.x + panelW, panel0.y + panelH);
    game::ui::drawPanel(registry, draw, panel0, panel1);

    game::ui::drawText(
        registry, draw,
        ImVec2(panel0.x + game::ui::kPauseTitleInset,
               panel0.y + game::ui::kPauseTitleInset),
        title, game::ui::kFontPanelTitle, IM_COL32_WHITE);

    const ImVec2 leftSize = game::ui::buttonSizeForLabel(registry, leftLabel);
    const ImVec2 rightSize = game::ui::buttonSizeForLabel(registry, rightLabel);
    const float m = game::ui::kPauseButtonMargin;
    const ImVec2 left0(panel0.x + m, panel1.y - m - leftSize.y);
    const ImVec2 right0(panel1.x - m - rightSize.x, panel1.y - m - rightSize.y);

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
    const ImVec2 canvas = game::ui::canvasSize(registry);
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    if (state != nullptr)
    {
        char scoreLabel[32];
        std::snprintf(scoreLabel, sizeof(scoreLabel), "SCORE: %06d",
                      state->score);
        game::ui::drawText(registry, draw,
                           ImVec2(game::ui::kScoreX, game::ui::kScoreY),
                           scoreLabel, game::ui::kFontScore, IM_COL32_WHITE);
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
        drawPauseLikePanel(registry, draw, canvas, "PAUSED", "RESUME", "QUIT",
                           focus, elapsed, confirm, left, right);
        if (left)
        {
            game::ui::playUi(registry);
            setPlayState(registry, PlayState::Playing);
        }
        else if (right)
        {
            game::ui::playUi(registry);
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
        drawPauseLikePanel(registry, draw, canvas, "GAME OVER", "RETRY", "QUIT",
                           focus, elapsed, confirm, left, right);
        if (left)
        {
            game::ui::playUi(registry);
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<InGameScene>());
        }
        else if (right)
        {
            game::ui::playUi(registry);
            registry.ctx().get<SceneSystem>().requestScene(
                std::make_unique<TitleScene>());
        }
    }
}

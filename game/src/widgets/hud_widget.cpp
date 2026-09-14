#include <cstdio>
#include <engine/scene/scene_system.hpp>
#include <game/play_state.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/ui/menu.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/hud_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;
using namespace game::ui;

namespace
{
enum class Choice
{
    None,
    Left,
    Right
};

/// The pause and game-over overlay: a centred panel with a title and two
/// buttons, focus moved with the arrows. Returns the button activated this
/// frame, if any.
Choice twoButtonPanel(entt::registry& registry, ImDrawList* draw, MenuNav& nav,
                      const char* title, const char* leftLabel,
                      const char* rightLabel)
{
    nav.update(registry, {"move_left", "move_up"}, {"move_right", "move_down"});

    const ImVec2 canvas = canvasSize(registry);
    const ImVec2 panel0((canvas.x - kPausePanelW) * 0.5f,
                        (canvas.y - kPausePanelH) * 0.5f);
    const ImVec2 panel1(panel0.x + kPausePanelW, panel0.y + kPausePanelH);
    drawPanel(registry, draw, panel0, panel1);
    drawText(registry, draw,
             ImVec2(panel0.x + kPauseTitleInset, panel0.y + kPauseTitleInset),
             title, kFontPanelTitle, IM_COL32_WHITE);

    const bool confirm = pressed(registry, "confirm");
    const ImVec2 leftSize = buttonSizeForLabel(registry, leftLabel);
    const ImVec2 rightSize = buttonSizeForLabel(registry, rightLabel);
    const float m = kPauseButtonMargin;

    const bool left =
        menuButton(registry, draw, leftLabel, leftLabel,
                   ImVec2(panel0.x + m, panel1.y - m - leftSize.y),
                   nav.focused(0), confirm);
    const bool right = menuButton(
        registry, draw, rightLabel, rightLabel,
        ImVec2(panel1.x - m - rightSize.x, panel1.y - m - rightSize.y),
        nav.focused(1), confirm);

    if (left)
    {
        return Choice::Left;
    }
    return right ? Choice::Right : Choice::None;
}

} // namespace

void HudWidget::render(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    char scoreLabel[32];
    std::snprintf(scoreLabel, sizeof(scoreLabel), "SCORE: %06d", state.score);
    drawText(registry, draw, ImVec2(kScoreX, kScoreY), scoreLabel, kFontScore,
             IM_COL32_WHITE);

    if (state.playState == PlayState::Paused)
    {
        switch (twoButtonPanel(registry, draw, m_pauseNav, "PAUSED", "RESUME",
                               "QUIT"))
        {
            case Choice::Left:
                playUi(registry);
                setPlayState(registry, PlayState::Playing);
                break;
            case Choice::Right:
                playUi(registry);
                registry.ctx().get<SceneSystem>().requestScene(
                    std::make_unique<TitleScene>());
                break;
            case Choice::None:
                break;
        }
    }
    else if (state.playState == PlayState::GameOver)
    {
        switch (twoButtonPanel(registry, draw, m_gameOverNav, "GAME OVER",
                               "RETRY", "QUIT"))
        {
            case Choice::Left:
                playUi(registry);
                registry.ctx().get<SceneSystem>().requestScene(
                    std::make_unique<InGameScene>());
                break;
            case Choice::Right:
                playUi(registry);
                registry.ctx().get<SceneSystem>().requestScene(
                    std::make_unique<TitleScene>());
                break;
            case Choice::None:
                break;
        }
    }
}

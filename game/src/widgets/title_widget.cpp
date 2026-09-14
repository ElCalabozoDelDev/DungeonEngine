#include <cmath>
#include <engine/core/game_loop.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/options_scene.hpp>
#include <game/ui/menu.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/title_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;
using namespace game::ui;

namespace
{
enum Item
{
    Start,
    Options
};

void drawTitle(entt::registry& registry, ImDrawList* draw)
{
    BitmapFont* bm = font(registry);
    if (bm == nullptr || !bm->ok())
    {
        return;
    }
    const auto drawLine = [&](const char* text, ImVec2 center)
    {
        const ImVec2 size = bm->measure(text, kFontTitle);
        const ImVec2 topLeft(std::floor(center.x - size.x * 0.5f),
                             std::floor(center.y - size.y * 0.5f));
        const ImVec2 shadow = kTitleShadowOffset;
        bm->draw(draw, ImVec2(topLeft.x + shadow.x, topLeft.y + shadow.y), text,
                 kFontTitle, kTitleShadow);
        bm->draw(draw, topLeft, text, kFontTitle, IM_COL32_WHITE);
    };
    drawLine("Dungeon", kTitleDungeonCenter);
    drawLine("Slime", kTitleSlimeCenter);
}

} // namespace

void TitleWidget::render(entt::registry& registry)
{
    const ImVec2 canvas = canvasSize(registry);
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    drawMenuBackground(registry, draw, canvas, m_scroll);
    drawTitle(registry, draw);

    m_nav.update(registry, {"move_left", "move_up"},
                 {"move_right", "move_down"});
    if (pressed(registry, "pause"))
    {
        registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
    }

    const bool confirm = pressed(registry, "confirm");
    const ImVec2 startSize = buttonSizeForLabel(registry, "Start");
    const ImVec2 optionsSize = buttonSizeForLabel(registry, "Options");
    const float buttonsY = canvas.y - kTitleButtonMarginY;

    const bool start =
        menuButton(registry, draw, "title-start", "Start",
                   ImVec2(kTitleButtonMarginX, buttonsY - startSize.y),
                   m_nav.focused(Start), confirm);
    const bool options =
        menuButton(registry, draw, "title-options", "Options",
                   ImVec2(canvas.x - kTitleButtonMarginX - optionsSize.x,
                          buttonsY - optionsSize.y),
                   m_nav.focused(Options), confirm);

    auto& scenes = registry.ctx().get<SceneSystem>();
    if (start)
    {
        playUi(registry);
        scenes.requestScene(std::make_unique<InGameScene>());
    }
    if (options)
    {
        playUi(registry);
        scenes.requestScene(std::make_unique<OptionsScene>());
    }
}

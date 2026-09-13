#include <cmath>
#include <engine/core/delta_time.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/options_scene.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>
#include <game/widgets/title_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

void TitleWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    game::ui::ensureLoaded(registry);

    const ImVec2 canvas = game::ui::canvasSize(registry);
    auto [focus, setFocus] = h.use_state(0);
    auto [scroll, setScroll] = h.use_state(ImVec2(0.0f, 0.0f));
    const double elapsed = registry.ctx().contains<DeltaTime>()
                               ? registry.ctx().get<DeltaTime>().elapsed
                               : 0.0;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0, 0), canvas, game::ui::kClearColor);
    game::ui::drawScrollingPattern(registry, draw, canvas, scroll, setScroll);

    if (game::ui::BitmapFont* bm = game::ui::font(registry);
        bm != nullptr && bm->ok())
    {
        auto drawTitle = [&](const char* text, ImVec2 center)
        {
            const ImVec2 size = bm->measure(text, game::ui::kFontTitle);
            const ImVec2 topLeft(std::floor(center.x - size.x * 0.5f),
                                 std::floor(center.y - size.y * 0.5f));
            const ImVec2 shadow = game::ui::kTitleShadowOffset;
            bm->draw(draw, ImVec2(topLeft.x + shadow.x, topLeft.y + shadow.y),
                     text, game::ui::kFontTitle, game::ui::kTitleShadow);
            bm->draw(draw, topLeft, text, game::ui::kFontTitle, IM_COL32_WHITE);
        };
        drawTitle("Dungeon", game::ui::kTitleDungeonCenter);
        drawTitle("Slime", game::ui::kTitleSlimeCenter);
    }

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input != nullptr && actions != nullptr)
    {
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
        if (actions->wasPressedRaw(*input, "pause"))
        {
            registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
        }
    }

    const ImVec2 startSize = game::ui::buttonSizeForLabel(registry, "Start");
    const ImVec2 optionsSize =
        game::ui::buttonSizeForLabel(registry, "Options");
    const ImVec2 start0(game::ui::kTitleButtonMarginX,
                        canvas.y - game::ui::kTitleButtonMarginY - startSize.y);
    const ImVec2 options0(
        canvas.x - game::ui::kTitleButtonMarginX - optionsSize.x,
        canvas.y - game::ui::kTitleButtonMarginY - optionsSize.y);

    game::ui::drawButton(registry, draw, start0, startSize, focus == 0,
                         elapsed);
    game::ui::drawCenteredText(
        registry, draw, start0,
        ImVec2(start0.x + startSize.x, start0.y + startSize.y), "Start",
        game::ui::kFontButton, game::ui::kButtonTextColor);
    game::ui::drawButton(registry, draw, options0, optionsSize, focus == 1,
                         elapsed);
    game::ui::drawCenteredText(
        registry, draw, options0,
        ImVec2(options0.x + optionsSize.x, options0.y + optionsSize.y),
        "Options", game::ui::kFontButton, game::ui::kButtonTextColor);

    const bool confirm = input != nullptr && actions != nullptr &&
                         actions->wasPressedRaw(*input, "confirm");
    if (game::ui::hitButton("title-start", start0, startSize) ||
        (confirm && focus == 0))
    {
        game::ui::playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<InGameScene>());
    }
    if (game::ui::hitButton("title-options", options0, optionsSize) ||
        (confirm && focus == 1))
    {
        game::ui::playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<OptionsScene>());
    }
}

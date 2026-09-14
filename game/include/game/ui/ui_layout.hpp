#ifndef GAME_UI_UI_LAYOUT_HPP
#define GAME_UI_UI_LAYOUT_HPP

#include <entt/entt.hpp>
#include <imgui.h>

namespace game::ui
{
/// Full-screen menus (Title / Options) use BackgroundDrawList.
/// In-game HUD overlays use ForegroundDrawList.

ImVec2 canvasSize(entt::registry& registry);
float presentScale(entt::registry& registry);

void playUi(entt::registry& registry);

float wrapPositive(float value, float period);

/// The full-screen menu backdrop: the clear colour, then the background
/// pattern tiled and scrolling diagonally (MonoGame title/options).
/// `scroll` is the widget's own offset, advanced by this frame's delta.
void drawMenuBackground(entt::registry& registry, ImDrawList* draw,
                        ImVec2 canvas, ImVec2& scroll);

} // namespace game::ui

#endif // GAME_UI_UI_LAYOUT_HPP

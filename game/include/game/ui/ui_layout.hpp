#ifndef GAME_UI_UI_LAYOUT_HPP
#define GAME_UI_UI_LAYOUT_HPP

#include <entt/entt.hpp>
#include <functional>
#include <imgui.h>

namespace game::ui
{
/// Full-screen menus (Title / Options) use BackgroundDrawList.
/// In-game HUD overlays use ForegroundDrawList.

ImVec2 canvasSize(entt::registry& registry);
float presentScale(entt::registry& registry);

void playUi(entt::registry& registry);

float wrapPositive(float value, float period);

/// Scrolling PointWrap background pattern (MonoGame title/options).
void drawScrollingPattern(entt::registry& registry, ImDrawList* draw,
                          ImVec2 canvas, ImVec2 scroll,
                          const std::function<void(const ImVec2&)>& setScroll);

} // namespace game::ui

#endif // GAME_UI_UI_LAYOUT_HPP

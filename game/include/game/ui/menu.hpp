#ifndef GAME_UI_MENU_HPP
#define GAME_UI_MENU_HPP

#include <entt/entt.hpp>
#include <imgui.h>
#include <initializer_list>
#include <string_view>

namespace game::ui
{
/// Keyboard focus over a row or column of menu items.
///
/// Every menu moves focus with the arrow keys and plays the UI sound when it
/// does; this is that behaviour once, instead of once per widget.
class MenuNav
{
public:
    enum class Ends
    {
        /// Stop at the first and last item (a two-button row).
        Clamp,
        /// Moving past the last item comes back to the first.
        Wrap,
    };

    MenuNav(int count, Ends ends) : m_count(count), m_ends(ends) {}

    int focus() const { return m_focus; }
    bool focused(int item) const { return m_focus == item; }

    /// Applies this frame's key presses: any action in `back` moves focus one
    /// item back, any in `forward` one item on. Plays the UI sound for each
    /// press, even one that cannot move further, as the menus always did.
    void update(entt::registry& registry,
                std::initializer_list<std::string_view> back,
                std::initializer_list<std::string_view> forward);

private:
    void step(int delta);

    int m_count;
    Ends m_ends;
    int m_focus = 0;
};

/// True on the frame `action` was pressed, even while Dear ImGui has the
/// keyboard (see ActionMap::wasPressedRaw). False when there is no input.
bool pressed(entt::registry& registry, std::string_view action);

/// Seconds since the loop started, for animating focus; 0 without a clock.
double uiClock(entt::registry& registry);

/// Draws a labelled button with its top-left corner at `topLeft` and returns
/// true when it is clicked, or when `confirm` is set while it has focus.
/// `id` must be unique among the buttons on screen.
///
/// Size it with buttonSizeForLabel() first when its position depends on it.
bool menuButton(entt::registry& registry, ImDrawList* draw, const char* id,
                std::string_view label, ImVec2 topLeft, bool focused,
                bool confirm);

} // namespace game::ui

#endif // GAME_UI_MENU_HPP

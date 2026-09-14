#ifndef GAME_WIDGETS_TITLE_WIDGET_HPP
#define GAME_WIDGETS_TITLE_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <game/ui/menu.hpp>
#include <imgui.h>

/// The title screen: the game's name over a scrolling backdrop, with Start
/// and Options. Esc quits.
class TitleWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry) override;

private:
    game::ui::MenuNav m_nav{2, game::ui::MenuNav::Ends::Clamp};
    ImVec2 m_scroll{0.0f, 0.0f};
};

#endif // GAME_WIDGETS_TITLE_WIDGET_HPP

#ifndef GAME_WIDGETS_OPTIONS_WIDGET_HPP
#define GAME_WIDGETS_OPTIONS_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <game/ui/menu.hpp>
#include <imgui.h>

/// The options screen: music and sound-effect volume sliders and Back.
/// Up/down move between them, left/right change a volume, Esc goes back.
class OptionsWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry) override;

private:
    game::ui::MenuNav m_nav{3, game::ui::MenuNav::Ends::Wrap};
    ImVec2 m_scroll{0.0f, 0.0f};
};

#endif // GAME_WIDGETS_OPTIONS_WIDGET_HPP

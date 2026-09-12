#ifndef GAME_WIDGETS_MENU_WIDGET_HPP
#define GAME_WIDGETS_MENU_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <entt/entt.hpp>

/// Title screen UI: start the level, or quit.
class MenuWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry, de::gui::Hooks& h) override;
};

#endif // GAME_WIDGETS_MENU_WIDGET_HPP

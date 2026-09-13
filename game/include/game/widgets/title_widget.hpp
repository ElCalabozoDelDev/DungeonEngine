#ifndef GAME_WIDGETS_TITLE_WIDGET_HPP
#define GAME_WIDGETS_TITLE_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <entt/entt.hpp>

class TitleWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry, de::gui::Hooks& h) override;
};

#endif // GAME_WIDGETS_TITLE_WIDGET_HPP

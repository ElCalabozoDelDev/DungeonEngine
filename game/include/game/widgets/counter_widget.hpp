#ifndef GAME_WIDGETS_COUNTER_WIDGET_HPP
#define GAME_WIDGETS_COUNTER_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <entt/entt.hpp>

/// Example widget showing the hook-based GUI layer (state, callback, effect).
class CounterWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry, de::gui::Hooks& h) override;
};

#endif // GAME_WIDGETS_COUNTER_WIDGET_HPP

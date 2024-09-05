#ifndef COUNTER_GUI_HPP
#define COUNTER_GUI_HPP
#include "widgets/gui.hpp"
#include "entt/entt.hpp"

class CounterWidget : public gui::WidgetComponent {
public:
    void render(entt::registry &registry, gui::Hooks &h) override;
};

#endif // COUNTER_GUI_HPP
#ifndef BOTTOM_GUI_HPP
#define BOTTOM_GUI_HPP

#include "widgets/gui.hpp"
#include "entt/entt.hpp"

class BottomGuiWidget : public gui::WidgetComponent {
public:
    void render(entt::registry &registry, gui::Hooks &h) override;
};

#endif // BOTTOM_GUI_HPP
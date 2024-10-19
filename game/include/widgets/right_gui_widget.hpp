#ifndef RIGHT_GUI_HPP
#define RIGHT_GUI_HPP

#include "widgets/gui.hpp"
#include "entt/entt.hpp"

class RightGuiWidget : public gui::WidgetComponent {
public:
    void render(entt::registry &registry, gui::Hooks &h) override;
};

#endif // RIGHT_GUI_HPP
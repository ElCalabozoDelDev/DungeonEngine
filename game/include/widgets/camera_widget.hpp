#ifndef CAMERA_WIDGET_HPP
#define CAMERA_WIDGET_HPP
#include "widgets/gui.hpp"
#include "entt/entt.hpp"

class CameraWidget : public gui::WidgetComponent {
public:
    void render(entt::registry &registry, gui::Hooks &h) override;
};

#endif // CAMERA_WIDGET_HPP
#include "widgets/right_gui_widget.hpp"
#include "imgui.h"

void RightGuiWidget::render(entt::registry &registry, gui::Hooks &h) {
    ImGui::Begin("Right GUI");
    ImGui::Text("This is the right GUI widget");
    ImGui::End();
}
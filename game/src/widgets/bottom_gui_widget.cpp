#include "widgets/bottom_gui_widget.hpp"
#include "imgui.h"

void BottomGuiWidget::render(entt::registry &registry, gui::Hooks &h) {
    ImGui::Begin("Bottom GUI");
    ImGui::Text("This is the bottom GUI widget");
    ImGui::End();
}
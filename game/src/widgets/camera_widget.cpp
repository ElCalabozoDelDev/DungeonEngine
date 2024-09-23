#include "widgets/camera_widget.hpp"
#include "components/camera_component.hpp"
#include "components/follow_component.hpp"
#include "components/position_component.hpp"
#include "imgui.h"

void CameraWidget::render(entt::registry &registry, gui::Hooks &h) {
    ImGui::Begin("Camera");
    auto view = registry.view<CameraComponent, FollowComponent, PositionComponent>();
    for (auto entity : view) {
        auto& camera = view.get<CameraComponent>(entity);
        auto& follow = view.get<FollowComponent>(entity);
        auto& position = view.get<PositionComponent>(entity);
        auto& targetPosition = registry.get<PositionComponent>(follow.target).position;
        ImGui::Text("Camera");
        ImGui::SliderFloat("Follow Speed", &camera.followSpeed, 0.0f, camera.followSpeed * 2);
        ImGui::Text("Target Position: (x: %f, y: %f)", targetPosition.getX(), targetPosition.getY());
        ImGui::Text("Position: (x: %f, y: %f)", position.position.m_x, position.position.m_y);
    }

    ImGui::End();
}
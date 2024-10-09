#pragma once

#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/player_component.hpp"
#include "components/transform_component.hpp"
#include "entt/entt.hpp"
#include "imgui_entt_entity_editor.hpp"
#include <entt/entity/fwd.hpp>

namespace MM {
template <>
inline void ComponentEditorWidget<PlayerComponent>(entt::registry &registry, entt::registry::entity_type entity) {
    auto &trf = registry.get<TransformComponent>(entity);
    float x = trf.position.getX();
    ImGui::InputFloat("X: ", &x);
    float y = trf.position.getY();
    ImGui::InputFloat("Y: ", &y);
}

template <>
inline void ComponentEditorWidget<CameraComponent>(entt::registry &registry, entt::registry::entity_type entity) {
    auto &trf = registry.get<TransformComponent>(entity);
    auto& bounds = registry.get<CameraBoundsComponent>(entity);
    float x = trf.position.getX();
    ImGui::InputFloat("X: ", &x);
    float y = trf.position.getY();
    ImGui::InputFloat("Y: ", &y);
    ImGui::InputFloat("Follow Speed: ", &registry.get<CameraComponent>(entity).followSpeed);
    float levelWidth = static_cast<float>(bounds.levelWidth);
    ImGui::InputFloat("Level Width: ", &levelWidth);
    float levelHeight = static_cast<float>(bounds.levelHeight);
    ImGui::InputFloat("Level Height: ", &levelHeight);
}

} // namespace MM

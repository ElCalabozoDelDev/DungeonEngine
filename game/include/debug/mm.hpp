#pragma once

#include "components/player_component.hpp"
#include "components/transform_component.hpp"
#include "entt/entt.hpp"
#include "imgui_entt_entity_editor.hpp"
#include <entt/entity/fwd.hpp>

namespace MM {
template <>
inline void ComponentEditorWidget<TransformComponent>(
    entt::registry &registry, entt::registry::entity_type entity) {
    auto &trf = registry.get<TransformComponent>(entity);
    ImGui::InputFloat("X: ", &trf.position.m_x);
    ImGui::InputFloat("Y: ", &trf.position.m_y);
}
} // namespace MM

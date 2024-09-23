#pragma once

#include "components/player_component.hpp"
#include "components/position_component.hpp"
#include "entt/entt.hpp"
#include "imgui_entt_entity_editor.hpp"
#include <entt/entity/fwd.hpp>

namespace MM {
template <>
inline void ComponentEditorWidget<PositionComponent>(
    entt::registry &registry, entt::registry::entity_type entity) {
    auto &pos = registry.get<PositionComponent>(entity);
    ImGui::InputFloat("X: ", &pos.position.m_x);
    ImGui::InputFloat("Y: ", &pos.position.m_y);
}
} // namespace MM

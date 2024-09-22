#include "systems/debug_system.hpp"
#include <entt/entity/fwd.hpp>

void DebugSystem::run(entt::registry& registry) {
    if (m_open) {
        // more on that later
        ImGui::Begin("Inspector");

        ImGui::Columns(2);
        ImGui::BeginChild("Entity List");
        m_editor.renderEntityList(registry, m_components_filter);
        ImGui::EndChild();
        ImGui::NextColumn();

        ImGui::BeginChild("Entity Editor");
        m_editor.renderEditor(registry, m_current_entity);
        ImGui::EndChild();
        ImGui::NextColumn();

        ImGui::End();

      }
}
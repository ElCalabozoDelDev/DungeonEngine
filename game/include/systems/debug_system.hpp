#ifndef DEBUG_SYSTEM_HPP
#define DEBUG_SYSTEM_HPP

#include "entt/entt.hpp"
#include "debug/mm.hpp"
#include "systems/system.hpp"
#include "entt/entity/fwd.hpp"

class DebugSystem final : public System {
    public:
        template<typename Component>
        void register_component(const std::string &display_name) {
            m_editor.registerComponent<Component>(display_name);
        }

        void run(entt::registry& registry) override;
    private:
        using Editor = MM::EntityEditor<entt::entity>;
        Editor m_editor;

        std::set<Editor::ComponentTypeID> m_components_filter;
        entt::entity m_current_entity;

        bool m_open{false};
};

#endif // DEBUG_SYSTEM_HPP
#ifndef DE_SYSTEMS_DEBUG_SYSTEM_HPP
#define DE_SYSTEMS_DEBUG_SYSTEM_HPP
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include <engine/core/debug.hpp>
#include <engine/systems/system.hpp>

namespace de
{
class DebugSystem final : public System
{
public:
    template <typename Component>
    void register_component(const std::string& display_name)
    {
        m_editor.registerComponent<Component>(display_name);
    }

    void run(entt::registry& registry) override;
    void setOpen(bool open) { m_open = open; }

private:
    using Editor = MM::EntityEditor<entt::entity>;
    Editor m_editor;

    std::set<Editor::ComponentTypeID> m_components_filter;
    entt::entity m_current_entity;

    bool m_open{false};
};

} // namespace de

#endif // DE_SYSTEMS_DEBUG_SYSTEM_HPP

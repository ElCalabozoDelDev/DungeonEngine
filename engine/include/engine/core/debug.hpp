#ifndef DE_CORE_DEBUG_HPP
#define DE_CORE_DEBUG_HPP

#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/transform_component.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <imgui_entt_entity_editor.hpp>

// Inspector widgets for the engine's own components. Games add their own by
// specializing MM::ComponentEditorWidget for their component types, and
// registering them with DebugSystem::register_component.
namespace MM
{
/// The editor only guarantees the entity has a CameraComponent. The transform
/// and the bounds are optional -- Dungeon Slime's fixed camera has no bounds,
/// and asking for them unconditionally crashed the inspector -- and edits are
/// written back rather than made to copies.
template <>
inline void
ComponentEditorWidget<de::CameraComponent>(entt::registry& registry,
                                           entt::registry::entity_type entity)
{
    auto& camera = registry.get<de::CameraComponent>(entity);

    if (auto* transform = registry.try_get<de::TransformComponent>(entity))
    {
        float x = transform->position.x;
        if (ImGui::InputFloat("X", &x))
        {
            transform->position.x = x;
        }
        float y = transform->position.y;
        if (ImGui::InputFloat("Y", &y))
        {
            transform->position.y = y;
        }
    }

    ImGui::InputFloat("Follow Speed", &camera.followSpeed);
    // Camera2D divides by the zoom, so a zero typed here must not stick.
    float zoom = camera.zoomLevel;
    if (ImGui::InputFloat("Zoom", &zoom) && zoom > 0.0f)
    {
        camera.zoomLevel = zoom;
    }

    if (auto* bounds = registry.try_get<de::CameraBoundsComponent>(entity))
    {
        ImGui::InputInt("Level Width", &bounds->levelWidth);
        ImGui::InputInt("Level Height", &bounds->levelHeight);
    }
}

} // namespace MM

#endif // DE_CORE_DEBUG_HPP

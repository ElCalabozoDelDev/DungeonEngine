#ifndef DE_CORE_DEBUG_HPP
#define DE_CORE_DEBUG_HPP

#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/transform_component.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <imgui_entt_entity_editor.hpp>

// Inspector widgets for the engine's own components. Games add their own by
// specializing MM::ComponentEditorWidget for their component types; see
// game/include/game/debug/player_editor.hpp for an example.
namespace MM
{
template <>
inline void
ComponentEditorWidget<de::CameraComponent>(entt::registry& registry,
                                           entt::registry::entity_type entity)
{
    auto& trf = registry.get<de::TransformComponent>(entity);
    auto& bounds = registry.get<de::CameraBoundsComponent>(entity);
    float x = trf.position.getX();
    ImGui::InputFloat("X: ", &x);
    float y = trf.position.getY();
    ImGui::InputFloat("Y: ", &y);
    ImGui::InputFloat("Follow Speed: ",
                      &registry.get<de::CameraComponent>(entity).followSpeed);
    float levelWidth = static_cast<float>(bounds.levelWidth);
    ImGui::InputFloat("Level Width: ", &levelWidth);
    float levelHeight = static_cast<float>(bounds.levelHeight);
    ImGui::InputFloat("Level Height: ", &levelHeight);
}

} // namespace MM

#endif // DE_CORE_DEBUG_HPP

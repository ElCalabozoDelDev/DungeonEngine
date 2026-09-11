#ifndef GAME_DEBUG_PLAYER_EDITOR_HPP
#define GAME_DEBUG_PLAYER_EDITOR_HPP

#include <engine/components/transform_component.hpp>
#include <engine/core/debug.hpp>
#include <game/components/player_component.hpp>

// Game-side inspector widget, registered by InGameScene. Kept out of the
// engine so the engine never has to know PlayerComponent exists.
namespace MM
{
template <>
inline void
ComponentEditorWidget<PlayerComponent>(entt::registry& registry,
                                       entt::registry::entity_type entity)
{
    auto& trf = registry.get<de::TransformComponent>(entity);
    float x = trf.position.getX();
    ImGui::InputFloat("X: ", &x);
    float y = trf.position.getY();
    ImGui::InputFloat("Y: ", &y);
}

} // namespace MM

#endif // GAME_DEBUG_PLAYER_EDITOR_HPP

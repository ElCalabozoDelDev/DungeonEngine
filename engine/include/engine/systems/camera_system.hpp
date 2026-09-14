#ifndef DE_SYSTEMS_CAMERA_SYSTEM_HPP
#define DE_SYSTEMS_CAMERA_SYSTEM_HPP
#include "entt/entt.hpp"
#include <engine/systems/system.hpp>

namespace de
{
/// Moves cameras that have a FollowComponent towards their target, clamped
/// to their CameraBoundsComponent.
///
/// Opt-in: BasePlugin does not register it. A game with a follow camera adds
/// it with addSystem().
class CameraSystem final : public System
{
public:
    void run(entt::registry& registry) override;
};

} // namespace de

#endif // DE_SYSTEMS_CAMERA_SYSTEM_HPP

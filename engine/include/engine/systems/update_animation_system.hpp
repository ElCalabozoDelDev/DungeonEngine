#ifndef DE_SYSTEMS_UPDATE_ANIMATION_SYSTEM_HPP
#define DE_SYSTEMS_UPDATE_ANIMATION_SYSTEM_HPP
#include "entt/entt.hpp"
#include <engine/systems/system.hpp>

namespace de
{
class UpdateAnimationSystem final : public System
{
public:
    void run(entt::registry& registry) override;
};

} // namespace de

#endif // DE_SYSTEMS_UPDATE_ANIMATION_SYSTEM_HPP

#ifndef DE_COMPONENTS_FOLLOW_COMPONENT_HPP
#define DE_COMPONENTS_FOLLOW_COMPONENT_HPP
#include "entt/entt.hpp"

namespace de
{
struct FollowComponent
{
    /// Entity the camera follows; null follows nothing.
    entt::entity target = entt::null;
};

} // namespace de

#endif // DE_COMPONENTS_FOLLOW_COMPONENT_HPP

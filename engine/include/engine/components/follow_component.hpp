#ifndef DE_COMPONENTS_FOLLOW_COMPONENT_HPP
#define DE_COMPONENTS_FOLLOW_COMPONENT_HPP
#include "entt/entt.hpp"

namespace de
{
struct FollowComponent
{
    entt::entity target; // Entity the camera follows
};

} // namespace de

#endif // DE_COMPONENTS_FOLLOW_COMPONENT_HPP

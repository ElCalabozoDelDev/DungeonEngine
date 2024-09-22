#ifndef FOLLOW_COMPONENT_HPP
#define FOLLOW_COMPONENT_HPP

#include "entt/entt.hpp"

struct FollowComponent
{
    entt::entity target; // Entidad que la cámara seguirá
};

#endif
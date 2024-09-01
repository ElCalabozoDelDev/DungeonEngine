#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "entt/entt.hpp"

class System {
public:
    virtual void run(entt::registry& registry) = 0;
    virtual ~System() = default;
};

#endif // SYSTEM_HPP
#ifndef DE_SYSTEMS_SYSTEM_HPP
#define DE_SYSTEMS_SYSTEM_HPP
#include "entt/entt.hpp"

namespace de
{
class System
{
public:
    virtual void run(entt::registry& registry) = 0;
    virtual ~System() = default;
};

} // namespace de

#endif // DE_SYSTEMS_SYSTEM_HPP

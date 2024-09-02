#ifndef UPDATE_POSITION_HPP
#define UPDATE_POSITION_HPP

#include "core/system.hpp"
#include "entt/entt.hpp"

class TransformSystem final : public System {
public:
    void run(entt::registry& registry) override;
};
#endif // UPDATE_POSITION_HPP
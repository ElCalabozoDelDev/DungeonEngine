#ifndef UPDATE_ANIMATION_SYSTEM_HPP
#define UPDATE_ANIMATION_SYSTEM_HPP

#include "core/system.hpp"
#include "entt/entt.hpp"
class UpdateAnimationSystem final : public System {
    public:
        void run(entt::registry& registry) override;
};
#endif // UPDATE_ANIMATION_SYSTEM_HPP
#ifndef CAMERA_SYSTEM_HPP
#define CAMERA_SYSTEM_HPP

#include "systems/system.hpp"
#include "entt/entt.hpp"

class CameraSystem final : public System {
    public:
        void run(entt::registry& registry) override;
};

#endif // CAMERA_SYSTEM_HPP
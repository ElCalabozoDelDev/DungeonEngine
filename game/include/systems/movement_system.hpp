#ifndef HANDLE_INPUT_HPP
#define HANDLE_INPUT_HPP

#include "core/system.hpp"
#include "entt/entt.hpp"

class MovementSystem final : public System {
    public:
        void run(entt::registry& registry) override;
};

#endif // HANDLE_INPUT_HPP
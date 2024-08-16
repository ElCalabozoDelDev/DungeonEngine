#include "systems/update_position.hpp"
#include "components/position.hpp"
#include "components/velocity.hpp"

void update_position(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& vel = view.get<Velocity>(entity);

        pos.x += vel.vx * deltaTime;
        pos.y += vel.vy * deltaTime;

        const int windowWidth = 800;
        const int windowHeight = 600;
        const int ballSize = 20;

        if (pos.x < 0) {
            pos.x = 0;
        } else if (pos.x + ballSize > windowWidth) {
            pos.x = windowWidth - ballSize;
        }

        if (pos.y < 0) {
            pos.y = 0;
        } else if (pos.y + ballSize > windowHeight) {
            pos.y = windowHeight - ballSize;
        }
    }
}
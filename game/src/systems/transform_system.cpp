#include "systems/transform_system.hpp"
#include "components/position_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"

void TransformSystem::run(entt::registry& registry) {
    DeltaTime * deltaTime = registry.ctx().get<DeltaTime *>();
    auto view = registry.view<PositionComponent, VelocityComponent>();
    for (auto entity : view) {
        auto& pos = view.get<PositionComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);

        pos.x += vel.vx * deltaTime->value;
        pos.y += vel.vy * deltaTime->value;

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
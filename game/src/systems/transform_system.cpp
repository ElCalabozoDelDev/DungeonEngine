#include "systems/transform_system.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "loaders/config.hpp"

void TransformSystem::run(entt::registry& registry) {
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    auto config = registry.ctx().get<Config>();

    auto view = registry.view<PositionComponent, VelocityComponent, SpriteComponent>();
    for (auto entity : view) {
        auto& pos = view.get<PositionComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        pos.position += vel.velocity * deltaTime.value;

        const int windowWidth = config.screenWidth;
        const int windowHeight = config.screenHeight;


        if(pos.position.getX() < 0) {
            pos.position.setX(0);
        } else if(pos.position.getX() + spr.spriteWidth > windowWidth) {
            pos.position.setX(windowWidth - spr.spriteWidth);
        }
        if(pos.position.getY() < 0) {
            pos.position.setY(0);
        } else if(pos.position.getY() + spr.spriteHeight > windowHeight) {
            pos.position.setY(windowHeight - spr.spriteHeight);
        }
    }
}
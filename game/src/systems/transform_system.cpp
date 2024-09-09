#include "systems/transform_system.hpp"
#include "components/position_component.hpp"
#include "components/texture_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "loaders/config.hpp"

void TransformSystem::run(entt::registry& registry) {
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    auto config = registry.ctx().get<Config>();

    auto view = registry.view<PositionComponent, VelocityComponent, TextureComponent>();
    for (auto entity : view) {
        auto& pos = view.get<PositionComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& tex = view.get<TextureComponent>(entity);

        pos.position += vel.velocity * deltaTime.value;

        const int windowWidth = config.screenWidth;
        const int windowHeight = config.screenHeight;


        if(pos.position.getX() < 0) {
            pos.position.setX(0);
        } else if(pos.position.getX() + tex.spriteWidth > windowWidth) {
            pos.position.setX(windowWidth - tex.spriteWidth);
        }
        if(pos.position.getY() < 0) {
            pos.position.setY(0);
        } else if(pos.position.getY() + tex.spriteHeight > windowHeight) {
            pos.position.setY(windowHeight - tex.spriteHeight);
        }
    }
}
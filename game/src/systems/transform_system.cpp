#include "systems/transform_system.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "loaders/config.hpp"
#include "core/quadtree.hpp"


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
        updateSpritePosition(registry, entity, pos.position);
    }
}

void TransformSystem::updateSpritePosition(entt::registry& registry, entt::entity entity, const Vector2D& newPosition) {
        auto& spriteQuadtree = registry.ctx().get<std::shared_ptr<SpriteQuadtree>>();

        // Verificar que la entidad sea válida antes de operar con ella
        if (!registry.valid(entity) || !registry.all_of<PositionComponent>(entity)) {
            return;  // La entidad no es válida, no se puede actualizar
        }

        // Primero eliminamos la entidad del Quadtree antiguo
        spriteQuadtree->remove(entity, registry.get<PositionComponent>(entity));

        // Actualizamos la posición de la entidad
        registry.get<PositionComponent>(entity).position = newPosition;

        // Insertamos la entidad de nuevo en el Quadtree con la nueva posición
        spriteQuadtree->insert(entity, registry.get<PositionComponent>(entity));
    }
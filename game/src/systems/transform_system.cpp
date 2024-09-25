#include "systems/transform_system.hpp"
#include "components/transform_component.hpp"
#include "components/sprite_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "loaders/config.hpp"
#include "core/quadtree.hpp"


void TransformSystem::run(entt::registry& registry) {
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    auto config = registry.ctx().get<Config>();

    auto view = registry.view<TransformComponent, VelocityComponent, SpriteComponent>();
    for (auto entity : view) {
        auto& trf = view.get<TransformComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        trf.position += vel.velocity * deltaTime.value;

        const int windowWidth = config.screenWidth;
        const int windowHeight = config.screenHeight;


        if(trf.position.getX() < 0) {
            trf.position.setX(0);
        } else if(trf.position.getX() + spr.spriteWidth > windowWidth) {
            trf.position.setX(windowWidth - spr.spriteWidth);
        }
        if(trf.position.getY() < 0) {
            trf.position.setY(0);
        } else if(trf.position.getY() + spr.spriteHeight > windowHeight) {
            trf.position.setY(windowHeight - spr.spriteHeight);
        }
        updateSpritePosition(registry, entity, trf.position);
    }
}

void TransformSystem::updateSpritePosition(entt::registry& registry, entt::entity entity, const Vector2D& newPosition) {
        auto& spriteQuadtree = registry.ctx().get<std::shared_ptr<SpriteQuadtree>>();

        // Verificar que la entidad sea válida antes de operar con ella
        if (!registry.valid(entity) || !registry.all_of<TransformComponent>(entity)) {
            return;  // La entidad no es válida, no se puede actualizar
        }

        // Primero eliminamos la entidad del Quadtree antiguo
        spriteQuadtree->remove(entity, registry.get<TransformComponent>(entity));

        // Actualizamos la posición de la entidad
        registry.get<TransformComponent>(entity).position = newPosition;

        // Insertamos la entidad de nuevo en el Quadtree con la nueva posición
        spriteQuadtree->insert(entity, registry.get<TransformComponent>(entity));
    }
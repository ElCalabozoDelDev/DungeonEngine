#include "systems/transform_system.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/transform_component.hpp"
#include "components/sprite_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "loaders/config.hpp"
#include "core/quadtree.hpp"


void TransformSystem::run(entt::registry& registry) {
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    auto config = registry.ctx().get<Config>();

    auto camView = registry.view<CameraBoundsComponent>();
    auto cameraEntity = *camView.begin();
    auto& camera = registry.get<CameraBoundsComponent>(cameraEntity);

    auto view = registry.view<TransformComponent, VelocityComponent, SpriteComponent>();
    for (auto entity : view) {
        auto& trf = view.get<TransformComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        trf.position += vel.velocity * deltaTime.value;

        const int levelWidth = camera.levelWidth;
        const int levelHeight = camera.levelHeight;


        if(trf.position.getX() < 0) {
            trf.position.setX(0);
        } else if(trf.position.getX() + spr.spriteWidth > levelWidth) {
            trf.position.setX(levelWidth - spr.spriteWidth);
        }
        if(trf.position.getY() < 0) {
            trf.position.setY(0);
        } else if(trf.position.getY() + spr.spriteHeight > levelHeight) {
            trf.position.setY(levelHeight - spr.spriteHeight);
        }
        updateSpritePosition(registry, entity, trf.position);
    }
}

void TransformSystem::updateSpritePosition(entt::registry& registry, entt::entity entity, const Vector2D& newPosition) {
        auto& objectQuadtree = registry.ctx().get<std::shared_ptr<ObjectQuadtree>>();

        // Verificar que la entidad sea válida antes de operar con ella
        if (!registry.valid(entity) || !registry.all_of<TransformComponent>(entity)) {
            return;  // La entidad no es válida, no se puede actualizar
        }

        // Primero eliminamos la entidad del Quadtree antiguo
        objectQuadtree->remove(entity, registry.get<TransformComponent>(entity));

        // Actualizamos la posición de la entidad
        registry.get<TransformComponent>(entity).position = newPosition;

        // Insertamos la entidad de nuevo en el Quadtree con la nueva posición
        objectQuadtree->insert(entity, registry.get<TransformComponent>(entity));
    }
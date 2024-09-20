#include "systems/update_animation_system.hpp"
#include "components/animation_component.hpp"
#include "components/sprite_component.hpp"
#include "core/delta_time.hpp"

void UpdateAnimationSystem::run(entt::registry& registry) {
    auto view = registry.view<AnimationComponent, SpriteComponent>();
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    for (auto entity : view) {
        auto& ani = view.get<AnimationComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        // Actualizar el tiempo acumulado
        ani.timeSinceLastFrame += deltaTime.value;

        // Cambiar de frame si el tiempo acumulado supera el tiempo entre frames
        if (ani.timeSinceLastFrame >= ani.animationTime) {
            // Mover al siguiente frame
            ani.currentFrame++;
            // Si el frame actual excede el total de frames, reiniciar al primer frame
            if (ani.currentFrame >= ani.totalFrames) {
                ani.currentFrame = 0;
            }
            // Actualizar la columna del sprite
            spr.currentSprite = spr.currentCol + ani.currentFrame;
            // Reiniciar el tiempo acumulado
            ani.timeSinceLastFrame = 0;
        }
    }
}
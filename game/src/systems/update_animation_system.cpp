#include "systems/update_animation_system.hpp"
#include "components/animation_component.hpp"
#include "components/texture_component.hpp"
#include "core/delta_time.hpp"

void UpdateAnimationSystem::run(entt::registry& registry) {
    auto view = registry.view<AnimationComponent, TextureComponent>();
    DeltaTime * deltaTime = registry.ctx().get<DeltaTime *>();
    for (auto entity : view) {
        auto& ani = view.get<AnimationComponent>(entity);
        auto& tex = view.get<TextureComponent>(entity);

        // Actualizar el tiempo acumulado
        ani.timeSinceLastFrame += deltaTime->value;
        // Cambiar de frame si el tiempo acumulado supera el tiempo entre frames
        if (ani.timeSinceLastFrame >= ani.animationTime) {
            // Mover al siguiente frame
            ani.currentFrame++;
            // Si el frame actual excede el total de frames, reiniciar al primer frame
            if (ani.currentFrame >= ani.totalFrames) {
                ani.currentFrame = 0;
            }
            // Actualizar la columna del sprite
            ani.currentSprite = tex.spriteCol + ani.currentFrame;
            // Reiniciar el tiempo acumulado
            ani.timeSinceLastFrame = 0;
        }
    }
}
#include "systems/update_animation_system.hpp"
#include "components/animation_component.hpp"
#include "components/texture_component.hpp"

void UpdateAnimationSystem::update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<AnimationComponent, TextureComponent>();

    for (auto entity : view) {
        auto& ani = view.get<AnimationComponent>(entity);
        auto& tex = view.get<TextureComponent>(entity);

        // Actualizar el tiempo acumulado
        ani.timeSinceLastFrame += deltaTime;
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
#include "systems/update_animation_system.hpp"
#include "components/texture_component.hpp"

void UpdateAnimationSystem::update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<TextureComponent>();

    for (auto entity : view) {
        auto& tex = view.get<TextureComponent>(entity);

        // Actualizar el tiempo acumulado
        tex.timeSinceLastFrame += deltaTime;
        // Cambiar de frame si el tiempo acumulado supera el tiempo entre frames
        if (tex.timeSinceLastFrame >= tex.animationTime) {
            // Mover al siguiente frame
            tex.currentFrame++;
            // Si el frame actual excede el total de frames, reiniciar al primer frame
            if (tex.currentFrame >= tex.totalFrames) {
                tex.currentFrame = 0;
            }
            // Actualizar la columna del sprite
            tex.currentSprite = tex.spriteCol + tex.currentFrame;
            // Reiniciar el tiempo acumulado
            tex.timeSinceLastFrame = 0;
        }
    }
}
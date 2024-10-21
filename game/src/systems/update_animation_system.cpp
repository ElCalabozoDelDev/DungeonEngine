#include "systems/update_animation_system.hpp"
#include "components/animation_component.hpp"
#include "components/sprite_component.hpp"
#include "core/delta_time.hpp"



#include <cmath> // Para funciones de suavizado

void UpdateAnimationSystem::run(entt::registry& registry) {
    auto view = registry.view<AnimationComponent, SpriteComponent>();
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();

    for (auto entity : view) {
        auto& ani = view.get<AnimationComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        // Ajustar la velocidad de la animación
        float adjustedDeltaTime = deltaTime.value * ani.speedMultiplier;

        // Actualizar el tiempo acumulado
        ani.timeSinceLastFrame += adjustedDeltaTime;

        // Verificar si la animación está pausada
        if (ani.isPaused) {
            ani.pauseTimer += adjustedDeltaTime;
            if (ani.pauseTimer >= ani.pauseDuration) {
                ani.isPaused = false;
                ani.pauseTimer = 0.0f;
            } else {
                continue;
            }
        }

        // Realizar interpolación si el tiempo acumulado es menor que el tiempo entre frames
        float interpolationFactor = ani.timeSinceLastFrame / ani.animationTime;

        // Aplicar diferentes tipos de interpolación
        switch (ani.interpolationMode) {
            case InterpolationMode::Linear:
                // Interpolación lineal simple
                spr.currentSprite = spr.currentCol + ani.currentFrame + interpolationFactor;
                break;

            case InterpolationMode::EaseIn:
                // Interpolación de aceleración suave (ease-in)
                interpolationFactor = std::pow(interpolationFactor, 2);
                spr.currentSprite = spr.currentCol + ani.currentFrame + interpolationFactor;
                break;

            case InterpolationMode::EaseOut:
                // Interpolación de desaceleración suave (ease-out)
                interpolationFactor = 1 - std::pow(1 - interpolationFactor, 2);
                spr.currentSprite = spr.currentCol + ani.currentFrame + interpolationFactor;
                break;

            case InterpolationMode::EaseInOut:
                // Interpolación de suavizado (ease-in-out)
                interpolationFactor = (interpolationFactor < 0.5) 
                    ? 2 * std::pow(interpolationFactor, 2) 
                    : 1 - std::pow(-2 * interpolationFactor + 2, 2) / 2;
                spr.currentSprite = spr.currentCol + ani.currentFrame + interpolationFactor;
                break;

            default:
                spr.currentSprite = spr.currentCol + ani.currentFrame;
                break;
        }

        // Cambiar de frame si el tiempo acumulado supera el tiempo entre frames
        if (ani.timeSinceLastFrame >= ani.animationTime) {
            switch (ani.loopType) {
                case AnimationLoopType::Oscillating:
                    if (ani.isReversing) {
                        ani.currentFrame--;
                        if (ani.currentFrame <= 0) {
                            ani.currentFrame = 0;
                            ani.isReversing = false;
                            if (ani.pauseAtEnds) ani.isPaused = true;
                        }
                    } else {
                        ani.currentFrame++;
                        if (ani.currentFrame >= ani.totalFrames - 1) {
                            ani.currentFrame = ani.totalFrames - 1;
                            ani.isReversing = true;
                            if (ani.pauseAtEnds) ani.isPaused = true;
                        }
                    }
                    break;

                case AnimationLoopType::Infinite:
                    ani.currentFrame++;
                    if (ani.currentFrame >= ani.totalFrames) {
                        ani.currentFrame = 0;
                        if (ani.pauseAtEnds) ani.isPaused = true;
                    }
                    break;

                case AnimationLoopType::Single:
                    ani.currentFrame++;
                    if (ani.currentFrame >= ani.totalFrames) {
                        ani.currentFrame = ani.totalFrames - 1;
                        ani.isPaused = true;
                    }
                    break;

                default:
                    break;
            }

            // Reiniciar el tiempo acumulado
            ani.timeSinceLastFrame = 0;
        }
    }
}

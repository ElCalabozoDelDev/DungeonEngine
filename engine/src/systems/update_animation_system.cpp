#include <engine/components/animation_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/systems/update_animation_system.hpp>

namespace de
{
void UpdateAnimationSystem::run(entt::registry& registry)
{
    auto view = registry.view<AnimationComponent, SpriteComponent>();
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    for (auto entity : view)
    {
        auto& ani = view.get<AnimationComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);

        // Accumulate elapsed time
        ani.timeSinceLastFrame += deltaTime.value;

        // Advance once the per-frame time has elapsed
        if (ani.timeSinceLastFrame >= ani.animationTime)
        {
            // Next frame
            ani.currentFrame++;
            // Si el frame actual excede el total de frames, reiniciar al primer
            // frame
            if (ani.currentFrame >= ani.totalFrames)
            {
                ani.currentFrame = 0;
            }
            // Update the sprite column
            spr.currentSprite = spr.currentCol + ani.currentFrame;
            // Reset the accumulator
            ani.timeSinceLastFrame = 0;
        }
    }
}

} // namespace de

#include <engine/components/animation_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/systems/update_animation_system.hpp>

namespace de
{
namespace
{
/// Moves to the next frame according to the loop mode. Returns true when the
/// run has just reached an end, which is where `holdAtEnds` applies.
bool advance(AnimationComponent& animation)
{
    const int last = animation.totalFrames - 1;

    switch (animation.loop)
    {
        case AnimationLoop::Loop:
            ++animation.currentFrame;
            if (animation.currentFrame > last)
            {
                animation.currentFrame = 0;
                return true;
            }
            return false;

        case AnimationLoop::PingPong:
            if (animation.reversing)
            {
                --animation.currentFrame;
                if (animation.currentFrame <= 0)
                {
                    animation.currentFrame = 0;
                    animation.reversing = false;
                    return true;
                }
            }
            else
            {
                ++animation.currentFrame;
                if (animation.currentFrame >= last)
                {
                    animation.currentFrame = last;
                    animation.reversing = true;
                    return true;
                }
            }
            return false;

        case AnimationLoop::Once:
            ++animation.currentFrame;
            if (animation.currentFrame >= last)
            {
                animation.currentFrame = last;
                animation.finished = true;
                return true;
            }
            return false;
    }

    return false;
}

} // namespace

void UpdateAnimationSystem::run(entt::registry& registry)
{
    const float delta = registry.ctx().get<DeltaTime>().value;

    for (auto&& [entity, animation, sprite] :
         registry.view<AnimationComponent, SpriteComponent>().each())
    {
        // A single-frame run has nothing to advance, and a zero frame time
        // would spin the loop below forever.
        if (animation.totalFrames <= 1 || animation.animationTime <= 0.0f)
        {
            sprite.currentSprite = sprite.currentCol + animation.currentFrame;
            continue;
        }

        if (animation.finished)
        {
            sprite.currentSprite = sprite.currentCol + animation.currentFrame;
            continue;
        }

        const float step = delta * animation.speedMultiplier;
        if (step <= 0.0f)
        {
            continue;
        }

        if (animation.holdRemaining > 0.0f)
        {
            animation.holdRemaining -= step;
            if (animation.holdRemaining > 0.0f)
            {
                continue;
            }
            animation.holdRemaining = 0.0f;
        }

        animation.timeSinceLastFrame += step;

        // A `while`, because one long frame may be worth several animation
        // frames. The remainder is carried rather than discarded: resetting
        // the accumulator to zero, as this used to, makes every run slightly
        // slower than the frame time asks for.
        while (animation.timeSinceLastFrame >= animation.animationTime)
        {
            animation.timeSinceLastFrame -= animation.animationTime;

            if (advance(animation) && animation.holdAtEnds > 0.0f)
            {
                animation.holdRemaining = animation.holdAtEnds;
                break;
            }
            if (animation.finished)
            {
                break;
            }
        }

        sprite.currentSprite = sprite.currentCol + animation.currentFrame;
    }
}

} // namespace de

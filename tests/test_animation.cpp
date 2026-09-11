#include <doctest/doctest.h>
#include <engine/components/animation_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/systems/update_animation_system.hpp>
#include <entt/entt.hpp>
#include <vector>

using namespace de;

namespace
{
constexpr float FrameTime = 0.1f;

/// A registry holding one animated sprite, and a helper to step it.
struct Animated
{
    entt::registry registry;
    entt::entity entity;
    UpdateAnimationSystem system;

    Animated(int totalFrames, AnimationLoop loop, int startColumn = 0)
    {
        registry.ctx().emplace<DeltaTime>();

        entity = registry.create();
        auto& animation = registry.emplace<AnimationComponent>(entity);
        animation.totalFrames = totalFrames;
        animation.animationTime = FrameTime;
        animation.loop = loop;

        registry.emplace<SpriteComponent>(entity, 0, startColumn, startColumn);
    }

    AnimationComponent& animation()
    {
        return registry.get<AnimationComponent>(entity);
    }

    const SpriteComponent& sprite()
    {
        return registry.get<SpriteComponent>(entity);
    }

    /// Runs the system with `seconds` of frame time.
    void step(float seconds)
    {
        registry.ctx().get<DeltaTime>().value = seconds;
        system.run(registry);
    }

    /// One whole frame's worth, plus a hair to land past the boundary.
    void stepOneFrame() { step(FrameTime + 0.0001f); }

    /// The frame indices produced by `count` successive steps.
    std::vector<int> stepMany(int count)
    {
        std::vector<int> frames;
        for (int i = 0; i < count; ++i)
        {
            stepOneFrame();
            frames.push_back(animation().currentFrame);
        }
        return frames;
    }
};

} // namespace

TEST_CASE("Loop runs 0,1,2 and starts again")
{
    Animated a(3, AnimationLoop::Loop);
    CHECK(a.stepMany(7) == std::vector<int>{1, 2, 0, 1, 2, 0, 1});
}

TEST_CASE("PingPong walks to the end and back")
{
    Animated a(3, AnimationLoop::PingPong);
    CHECK(a.stepMany(8) == std::vector<int>{1, 2, 1, 0, 1, 2, 1, 0});
}

TEST_CASE("Once stops on the last frame")
{
    Animated a(3, AnimationLoop::Once);

    CHECK(a.stepMany(5) == std::vector<int>{1, 2, 2, 2, 2});
    CHECK(a.animation().finished);

    SUBCASE("and restarts when the flag is cleared")
    {
        a.animation().finished = false;
        a.animation().currentFrame = 0;
        a.stepOneFrame();
        CHECK(a.animation().currentFrame == 1);
    }
}

TEST_CASE("the sprite column is the run start plus the frame")
{
    // currentFrame is an offset from currentCol. Seeding it with the column,
    // as the loader used to, drew 2 * column on the first frame.
    Animated a(3, AnimationLoop::Loop, 8);
    CHECK(a.sprite().currentSprite == 8);

    a.stepOneFrame();
    CHECK(a.sprite().currentSprite == 9);
}

TEST_CASE("a long frame advances more than one animation frame")
{
    Animated a(4, AnimationLoop::Loop);
    a.step(FrameTime * 3.0f + 0.0001f);
    CHECK(a.animation().currentFrame == 3);
}

TEST_CASE("the leftover time is carried, not discarded")
{
    // Resetting the accumulator to zero makes every run slightly slower than
    // its frame time asks for.
    Animated a(4, AnimationLoop::Loop);
    a.step(FrameTime * 1.5f);
    REQUIRE(a.animation().currentFrame == 1);
    CHECK(a.animation().timeSinceLastFrame ==
          doctest::Approx(FrameTime * 0.5f));
}

TEST_CASE("speedMultiplier scales playback")
{
    SUBCASE("faster")
    {
        Animated a(4, AnimationLoop::Loop);
        a.animation().speedMultiplier = 2.0f;
        // One frame time at double speed is worth two animation frames.
        a.step(FrameTime);
        CHECK(a.animation().currentFrame == 2);
    }

    SUBCASE("frozen at zero")
    {
        Animated a(4, AnimationLoop::Loop);
        a.animation().speedMultiplier = 0.0f;
        a.step(FrameTime * 10.0f);
        CHECK(a.animation().currentFrame == 0);
    }
}

TEST_CASE("holdAtEnds pauses at each end of the run")
{
    Animated a(3, AnimationLoop::Loop);
    a.animation().holdAtEnds = 0.5f;

    a.stepOneFrame(); // -> 1
    a.stepOneFrame(); // -> 2
    a.stepOneFrame(); // -> 0, and starts holding
    REQUIRE(a.animation().currentFrame == 0);
    REQUIRE(a.animation().holdRemaining > 0.0f);

    SUBCASE("the frame does not advance while holding")
    {
        a.step(0.2f);
        CHECK(a.animation().currentFrame == 0);
        CHECK(a.animation().holdRemaining == doctest::Approx(0.3f));

        a.step(0.2f);
        CHECK(a.animation().currentFrame == 0);
        CHECK(a.animation().holdRemaining == doctest::Approx(0.1f));
    }

    SUBCASE("and resumes once the hold expires")
    {
        a.step(0.4f); // 0.5 - 0.4 leaves 0.1 of hold
        REQUIRE(a.animation().currentFrame == 0);

        a.step(0.15f); // clears the remaining hold, then one frame's worth
        CHECK(a.animation().holdRemaining == doctest::Approx(0.0f));
        CHECK(a.animation().currentFrame == 1);
    }
}

TEST_CASE("a single-frame run is left alone")
{
    // Guards the zero-division and the while loop: totalFrames of 1 has
    // nothing to advance to.
    Animated a(1, AnimationLoop::Loop);
    a.step(FrameTime * 100.0f);
    CHECK(a.animation().currentFrame == 0);
}

TEST_CASE("a zero frame time does not hang")
{
    Animated a(3, AnimationLoop::Loop);
    a.animation().animationTime = 0.0f;
    a.step(FrameTime);
    CHECK(a.animation().currentFrame == 0);
}

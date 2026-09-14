#include <algorithm>
#include <doctest/doctest.h>
#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/follow_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/camera_system.hpp>
#include <engine/systems/spatial_sync_system.hpp>
#include <engine/systems/transform_system.hpp>
#include <entt/entt.hpp>
#include <vector>

using namespace de;

TEST_CASE("TransformSystem integrates one fixed step, not one frame")
{
    // Regression test. It is a fixed system but moved entities by
    // DeltaTime::value, so a 50 ms frame split into three steps moved them
    // three frames' worth.
    entt::registry registry;
    registry.ctx().emplace<DeltaTime>(DeltaTime{0.05f, 1.0f / 60.0f});

    auto level = registry.create();
    registry.emplace<CameraBoundsComponent>(level, 1000, 1000);

    auto mover = registry.create();
    registry.emplace<TransformComponent>(mover,
                                         Vector2D<float>(100.0f, 100.0f));
    registry.emplace<VelocityComponent>(mover, Vector2D<float>(60.0f, 0.0f));
    registry.emplace<SpriteComponent>(mover, 0, 0, 0);
    registry.emplace<DimensionComponent>(mover, 10.0f, 10.0f);

    TransformSystem().run(registry);

    CHECK(registry.get<TransformComponent>(mover).position.getX() ==
          doctest::Approx(101.0f)); // 60 px/s for 1/60 s
}

TEST_CASE("CameraSystem follows a target that has no velocity")
{
    // Regression test. It asked for the target's VelocityComponent
    // unconditionally, which asserts for anything that does not move by
    // velocity.
    entt::registry registry;
    registry.ctx().emplace<DeltaTime>(DeltaTime{0.1f, 1.0f / 60.0f});

    auto target = registry.create();
    registry.emplace<TransformComponent>(target,
                                         Vector2D<float>(600.0f, 500.0f));

    auto camera = registry.create();
    registry.emplace<CameraComponent>(camera);
    registry.emplace<CameraBoundsComponent>(camera, 2000, 2000);
    registry.emplace<TransformComponent>(camera,
                                         Vector2D<float>(400.0f, 500.0f));
    registry.emplace<DimensionComponent>(camera, 320.0f, 180.0f);
    registry.emplace<FollowComponent>(camera, target);

    CameraSystem().run(registry);

    const float x = registry.get<TransformComponent>(camera).position.getX();
    CHECK(x > 400.0f);
    CHECK(x < 600.0f);

    SUBCASE("and skips a camera whose target is gone")
    {
        registry.destroy(target);
        CameraSystem().run(registry);
        CHECK(registry.get<TransformComponent>(camera).position.getX() ==
              doctest::Approx(x));
    }
}

TEST_CASE("SpatialSyncSystem re-files sprites that move without a velocity")
{
    // Regression test. It only looked at entities with a VelocityComponent;
    // the snake and the bat move themselves and stayed filed where they
    // spawned.
    entt::registry registry;
    auto& index = registry.ctx().emplace<SpatialIndex>();
    index.create(
        Layer::Object,
        [&registry](const entt::entity& entity)
        {
            const auto& p = registry.get<TransformComponent>(entity).position;
            return Box<float>(p.getX(), p.getY(), 10.0f, 10.0f);
        },
        Box<float>(0.0f, 0.0f, 1000.0f, 1000.0f));

    std::vector<entt::entity> all;
    for (int i = 0; i < 40; ++i)
    {
        auto e = registry.create();
        registry.emplace<TransformComponent>(
            e, Vector2D<float>(static_cast<float>((i % 8) * 100 + 10),
                               static_cast<float>((i / 8) * 100 + 10)));
        REQUIRE(index.insert(Layer::Object, e));
        all.push_back(e);
    }

    const entt::entity mover = all.front();
    registry.get<TransformComponent>(mover).position =
        Vector2D<float>(900.0f, 900.0f);

    SpatialSyncSystem().run(registry);

    const auto hits =
        index.query(Layer::Object, Box<float>(895.0f, 895.0f, 20.0f, 20.0f));
    CHECK(std::find(hits.begin(), hits.end(), mover) != hits.end());
}

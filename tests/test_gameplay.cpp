#include <doctest/doctest.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/solid_body_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/collision_system.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/state.hpp>
#include <game/systems/combat_system.hpp>
#include <game/systems/enemy_ai_system.hpp>

using namespace de;

namespace
{
/// A registry with the context the gameplay systems read, and a spatial index
/// over a small world. No window, no SDL: these systems only touch components.
struct World
{
    entt::registry registry;

    World()
    {
        registry.ctx().emplace<DeltaTime>(
            DeltaTime{1.0f / 60.0f, 1.0f / 60.0f, 0.0f, 0.0});
        registry.ctx().emplace<GameState>();

        auto& spatial = registry.ctx().emplace<SpatialIndex>();
        auto getBox = [this](const entt::entity& entity)
        {
            const auto& t = registry.get<TransformComponent>(entity);
            const auto& d = registry.get<DimensionComponent>(entity);
            return Box<float>(t.position.getX(), t.position.getY(), d.width,
                              d.height);
        };
        const Box<float> bounds(0, 0, 512, 512);
        spatial.create(Layer::Collision, getBox, bounds);
        spatial.create(Layer::Overlay, getBox, bounds);
        spatial.create(Layer::Object, getBox, bounds);
    }

    SpatialIndex& spatial() { return registry.ctx().get<SpatialIndex>(); }
    GameState& state() { return registry.ctx().get<GameState>(); }

    entt::entity spawn(float x, float y)
    {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<DimensionComponent>(entity, 16.0f, 16.0f);
        registry.emplace<VelocityComponent>(entity, Vector2D<float>(0, 0));
        return entity;
    }

    entt::entity spawnPlayer(float x, float y)
    {
        auto entity = spawn(x, y);
        registry.emplace<PlayerComponent>(entity);
        registry.emplace<HealthComponent>(entity);
        registry.emplace<SolidBodyComponent>(entity);
        spatial().insert(Layer::Object, entity);
        return entity;
    }

    Vector2D<float> positionOf(entt::entity entity)
    {
        return registry.get<TransformComponent>(entity).position;
    }
};

} // namespace

TEST_CASE("a solid body is pushed out of a wall")
{
    World world;

    auto wall = world.spawn(100, 100);
    world.spatial().insert(Layer::Collision, wall);

    // Overlapping the wall by 6px on X, aligned on Y.
    auto player = world.spawnPlayer(94, 100);

    CollisionSystem collision;
    collision.run(world.registry);

    SUBCASE("along the axis of least overlap")
    {
        CHECK(world.positionOf(player).getX() <= 84.0f + 0.001f);
    }

    SUBCASE("and not along the other one")
    {
        CHECK(world.positionOf(player).getY() == doctest::Approx(100.0f));
    }
}

TEST_CASE("an entity with no SolidBodyComponent passes through")
{
    World world;
    auto wall = world.spawn(100, 100);
    world.spatial().insert(Layer::Collision, wall);

    auto ghost = world.spawn(94, 100);
    const auto before = world.positionOf(ghost);

    CollisionSystem collision;
    collision.run(world.registry);

    CHECK(world.positionOf(ghost).getX() == doctest::Approx(before.getX()));
}

TEST_CASE("enemies chase the player only within range")
{
    World world;
    auto player = world.spawnPlayer(100, 100);
    (void)player;

    auto near = world.spawn(130, 100);
    world.registry.emplace<EnemyComponent>(near);

    auto far = world.spawn(460, 460);
    world.registry.emplace<EnemyComponent>(far);

    EnemyAISystem ai;
    ai.run(world.registry);

    const auto nearVelocity =
        world.registry.get<VelocityComponent>(near).velocity;
    const auto farVelocity =
        world.registry.get<VelocityComponent>(far).velocity;

    CHECK(nearVelocity.length() > 1.0f);
    CHECK(nearVelocity.getX() < 0.0f); // the player is to its left
    CHECK(farVelocity.length() == doctest::Approx(0.0f));
}

TEST_CASE("contact damage respects its cooldown")
{
    World world;
    auto player = world.spawnPlayer(100, 100);
    auto enemy = world.spawn(100, 100); // exactly on top of the player
    world.registry.emplace<EnemyComponent>(enemy);

    CombatSystem combat;
    const int startingHealth =
        world.registry.get<HealthComponent>(player).current;

    combat.run(world.registry);
    CHECK(world.registry.get<HealthComponent>(player).current ==
          startingHealth - 1);

    SUBCASE("no second hit while still invulnerable")
    {
        combat.run(world.registry);
        CHECK(world.registry.get<HealthComponent>(player).current ==
              startingHealth - 1);
    }

    SUBCASE("hit again once the cooldown expires")
    {
        // The cooldown is one second; each run advances one fixed step.
        for (int i = 0; i < 70; ++i)
        {
            combat.run(world.registry);
        }
        CHECK(world.registry.get<HealthComponent>(player).current <
              startingHealth - 1);
    }
}

TEST_CASE("an item is collected, destroyed, and removed from the index")
{
    World world;
    auto player = world.spawnPlayer(100, 100);
    (void)player;

    auto item = world.spawn(100, 100);
    world.registry.emplace<ItemComponent>(item);
    world.spatial().insert(Layer::Object, item);
    world.state().itemsTotal = 1;

    CombatSystem combat;
    combat.run(world.registry);

    CHECK(world.state().itemsCollected == 1);
    CHECK_FALSE(world.registry.valid(item));

    SUBCASE("the index does not keep a handle to the destroyed entity")
    {
        const auto hits =
            world.spatial().query(Layer::Object, Box<float>(96, 96, 24, 24));
        CHECK(std::find(hits.begin(), hits.end(), item) == hits.end());
    }
}

TEST_CASE("running out of health raises game over")
{
    World world;
    auto player = world.spawnPlayer(100, 100);
    auto enemy = world.spawn(100, 100);
    world.registry.emplace<EnemyComponent>(enemy);

    auto& health = world.registry.get<HealthComponent>(player);
    health.current = 1;

    CombatSystem combat;
    combat.run(world.registry);

    CHECK(health.current == 0); // floors, does not go negative
    CHECK(world.state().gameOver);
}

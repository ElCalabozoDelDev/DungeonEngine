#include <doctest/doctest.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/paused.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/state.hpp>
#include <game/systems/bat_system.hpp>
#include <game/systems/snake_system.hpp>

using namespace de;

namespace
{
struct World
{
    entt::registry registry;

    World()
    {
        registry.ctx().emplace<DeltaTime>(
            DeltaTime{1.0f / 60.0f, 1.0f / 60.0f, 0.0f, 0.0});
        registry.ctx().emplace<Paused>();
        auto& state = registry.ctx().emplace<GameState>();
        state.roomBounds = Rectangle{20.0f, 20.0f, 280.0f, 140.0f};
        state.playState = PlayState::Playing;

        registry.ctx().emplace<InputState>();
        registry.ctx().emplace<ActionMap>();
    }

    GameState& state() { return registry.ctx().get<GameState>(); }

    entt::entity spawnSnake(float x, float y, int segments = 3)
    {
        auto entity = registry.create();
        registry.emplace<PlayerComponent>(entity);
        SnakeComponent snake;
        snake.stride = 16.0f;
        for (int i = 0; i < segments; ++i)
        {
            SlimeSegment segment;
            segment.at = Vector2D<float>(x - static_cast<float>(i) * 16.0f, y);
            segment.to = segment.at;
            segment.direction = Vector2D<float>(1.0f, 0.0f);
            snake.segments.push_back(segment);
        }
        snake.nextDirection = Vector2D<float>(1.0f, 0.0f);
        registry.emplace<SnakeComponent>(entity, snake);
        registry.emplace<TransformComponent>(
            entity, Vector2D<float>(x - 8.0f, y - 8.0f));
        registry.emplace<DimensionComponent>(entity, 16.0f, 16.0f);
        return entity;
    }

    void step(int times = 1)
    {
        SnakeSystem snake;
        BatSystem bat;
        for (int i = 0; i < times; ++i)
        {
            snake.run(registry);
            bat.run(registry);
        }
    }
};

} // namespace

TEST_CASE("snake grows when pendingGrowth is set")
{
    World world;
    auto entity = world.spawnSnake(100.0f, 80.0f, 2);
    auto& snake = world.registry.get<SnakeComponent>(entity);
    snake.pendingGrowth = 1;

    // Advance past one movement interval (0.2s at 60Hz ≈ 12 steps).
    world.step(13);

    CHECK(snake.segments.size() == 3);
}

TEST_CASE("snake dies on self collision")
{
    World world;
    auto entity = world.spawnSnake(100.0f, 80.0f, 4);
    auto& snake = world.registry.get<SnakeComponent>(entity);

    // U-turn fold: keep the tail so the new head lands on it.
    snake.pendingGrowth = 1;
    snake.segments[0].to = Vector2D<float>(100.0f, 80.0f);
    snake.segments[1].to = Vector2D<float>(84.0f, 80.0f);
    snake.segments[2].to = Vector2D<float>(84.0f, 96.0f);
    snake.segments[3].to = Vector2D<float>(100.0f, 96.0f);
    snake.segments[0].direction = Vector2D<float>(0.0f, 1.0f);
    snake.nextDirection = Vector2D<float>(0.0f, 1.0f);

    world.step(13);

    CHECK(world.state().playState == PlayState::GameOver);
    CHECK(world.state().gameOver);
}

TEST_CASE("snake dies outside room bounds")
{
    World world;
    auto entity = world.spawnSnake(290.0f, 80.0f, 1);
    auto& snake = world.registry.get<SnakeComponent>(entity);
    snake.nextDirection = Vector2D<float>(1.0f, 0.0f);

    world.step(13);

    CHECK(world.state().playState == PlayState::GameOver);
}

TEST_CASE("eating a bat grows the snake and awards score")
{
    World world;
    auto snakeEntity = world.spawnSnake(100.0f, 80.0f, 2);
    auto& snake = world.registry.get<SnakeComponent>(snakeEntity);

    auto bat = world.registry.create();
    world.registry.emplace<BatComponent>(bat);
    world.registry.emplace<TransformComponent>(bat,
                                               Vector2D<float>(92.0f, 72.0f));
    world.registry.emplace<DimensionComponent>(bat, 16.0f, 16.0f);
    world.registry.emplace<VelocityComponent>(bat, Vector2D<float>(0, 0));

    BatSystem batSystem;
    batSystem.run(world.registry);

    CHECK(snake.pendingGrowth == 1);
    CHECK(world.state().score == SnakeComponent::scorePerBat);
}

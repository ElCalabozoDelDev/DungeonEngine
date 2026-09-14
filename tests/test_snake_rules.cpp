#include <SDL.h>
#include <array>
#include <cmath>
#include <doctest/doctest.h>
#include <engine/components/animation_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/snake_component.hpp>
#include <game/prefabs.hpp>
#include <game/systems/bat_system.hpp>
#include <game/systems/snake_system.hpp>
#include <initializer_list>
#include <random>
#include <string>

using namespace de;
using game::snake::StepOutcome;

namespace
{
const Box<float> Room(20.0f, 20.0f, 280.0f, 140.0f);

/// A snake whose head is at `head` and whose body trails to the left.
SnakeComponent snakeAt(Vector2D<float> head, int segments)
{
    SnakeComponent snake;
    snake.stride = 20.0f;
    for (int i = 0; i < segments; ++i)
    {
        SlimeSegment segment;
        segment.at =
            Vector2D<float>(head.x - static_cast<float>(i) * 20.0f, head.y);
        segment.to = segment.at;
        snake.segments.push_back(segment);
    }
    return snake;
}

struct Keys
{
    InputState input;
    ActionMap actions;

    explicit Keys(std::initializer_list<SDL_Scancode> held)
    {
        actions.bind("move_up", SDL_SCANCODE_UP);
        actions.bind("move_down", SDL_SCANCODE_DOWN);
        actions.bind("move_left", SDL_SCANCODE_LEFT);
        actions.bind("move_right", SDL_SCANCODE_RIGHT);
        std::array<Uint8, SDL_NUM_SCANCODES> keys{};
        for (auto key : held)
        {
            keys[static_cast<std::size_t>(key)] = 1;
        }
        input.setKeyboard(keys.data(), SDL_NUM_SCANCODES);
    }
};

} // namespace

TEST_CASE("steer turns but never reverses")
{
    const SnakeComponent snake = snakeAt(Vector2D<float>(100.0f, 80.0f), 3);
    const Vector2D<float> right(1.0f, 0.0f);

    CHECK(game::snake::steer(snake, Keys({}).input, Keys({}).actions) == right);

    Keys up({SDL_SCANCODE_UP});
    CHECK(game::snake::steer(snake, up.input, up.actions) ==
          Vector2D<float>(0.0f, -1.0f));

    Keys back({SDL_SCANCODE_LEFT});
    CHECK(game::snake::steer(snake, back.input, back.actions) == right);
}

TEST_CASE("advance moves the head one stride and drops the tail")
{
    SnakeComponent snake = snakeAt(Vector2D<float>(100.0f, 80.0f), 3);

    CHECK(game::snake::advance(snake, Room) == StepOutcome::Moved);
    REQUIRE(snake.segments.size() == 3);
    CHECK(snake.segments.front().at == Vector2D<float>(100.0f, 80.0f));
    CHECK(snake.segments.front().to == Vector2D<float>(120.0f, 80.0f));
    CHECK(snake.segments.back().to == Vector2D<float>(80.0f, 80.0f));

    SUBCASE("and keeps the tail while growth is pending")
    {
        snake.pendingGrowth = 1;
        CHECK(game::snake::advance(snake, Room) == StepOutcome::Moved);
        CHECK(snake.segments.size() == 4);
        CHECK(snake.pendingGrowth == 0);
    }
}

TEST_CASE("advance stops on the last floor cell at a wall")
{
    SnakeComponent snake = snakeAt(Vector2D<float>(290.0f, 80.0f), 2);

    CHECK(game::snake::advance(snake, Room) == StepOutcome::HitWall);
    CHECK(snake.segments.size() == 2);
    CHECK(snake.segments.front().to == Vector2D<float>(290.0f, 80.0f));
    CHECK(snake.segments.front().at == snake.segments.front().to);
}

TEST_CASE("advance reports running into the body")
{
    // Head at (100,80) heading down, body curling round to sit below it.
    SnakeComponent snake;
    snake.stride = 20.0f;
    for (const auto& cell :
         {Vector2D<float>(100.0f, 80.0f), Vector2D<float>(80.0f, 80.0f),
          Vector2D<float>(80.0f, 100.0f), Vector2D<float>(100.0f, 100.0f)})
    {
        SlimeSegment segment;
        segment.at = cell;
        segment.to = cell;
        snake.segments.push_back(segment);
    }
    snake.pendingGrowth = 1; // keep the tail where the head is going
    snake.nextDirection = Vector2D<float>(0.0f, 1.0f);

    CHECK(game::snake::advance(snake, Room) == StepOutcome::HitSelf);
}

TEST_CASE("makeSnakeHead turns a Tiled object into the snake head")
{
    entt::registry registry;
    auto entity = registry.create();
    registry.emplace<ObjectTypeComponent>(entity, "Player");
    registry.emplace<TransformComponent>(entity, Vector2D<float>(3.0f, 4.0f));
    registry.emplace<DimensionComponent>(entity, 32.0f, 32.0f);

    game::prefab::makeSnakeHead(registry, entity, Vector2D<float>(50.0f, 70.0f),
                                20.0f);

    CHECK(registry.all_of<PlayerComponent, SpriteComponent>(entity));
    const auto& snake = registry.get<SnakeComponent>(entity);
    CHECK(snake.stride == doctest::Approx(20.0f));
    REQUIRE(snake.segments.size() == 1);
    CHECK(snake.segments.front().to == Vector2D<float>(50.0f, 70.0f));
    CHECK(registry.get<DimensionComponent>(entity).width ==
          doctest::Approx(20.0f));
    CHECK(registry.get<TransformComponent>(entity).position ==
          Vector2D<float>(40.0f, 60.0f));
    CHECK(registry.get<TextureComponent>(entity).id ==
          game::prefab::SlimeStrip.texture);
    CHECK(registry.get<AnimationComponent>(entity).totalFrames ==
          game::prefab::SlimeStrip.frames);
    CHECK(registry.all_of<ObjectTypeComponent>(entity)); // left in place
}

TEST_CASE("makeBat draws heading, then x, then y")
{
    // The draw order is what makes a seed reproducible; before, x and y were
    // two arguments of one call and their order was up to the compiler.
    entt::registry registry;
    std::mt19937 rng(42);
    auto entity = game::prefab::makeBat(registry, Room, rng);

    std::mt19937 replay(42);
    const Vector2D<float> velocity =
        game::bat::randomVelocity(BatComponent{}.speed, replay);
    std::uniform_real_distribution<float> xDist(20.0f, 300.0f - 20.0f);
    std::uniform_real_distribution<float> yDist(20.0f, 160.0f - 20.0f);
    const float x = xDist(replay);
    const float y = yDist(replay);

    CHECK(registry.get<BatComponent>(entity).velocity == velocity);
    CHECK(registry.get<TransformComponent>(entity).position ==
          Vector2D<float>(x, y));
    CHECK(registry.get<TextureComponent>(entity).id ==
          game::prefab::BatStrip.texture);
}

TEST_CASE("randomVelocity has the requested speed and draws once")
{
    std::mt19937 rng(7);
    const auto velocity = game::bat::randomVelocity(75.0f, rng);
    CHECK(velocity.length() == doctest::Approx(75.0f));

    std::mt19937 skipped(7);
    skipped.discard(1);
    CHECK(rng() == skipped());
}

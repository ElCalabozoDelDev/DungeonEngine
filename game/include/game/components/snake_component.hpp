#ifndef GAME_COMPONENTS_SNAKE_COMPONENT_HPP
#define GAME_COMPONENTS_SNAKE_COMPONENT_HPP

#include <engine/core/vector_2d.hpp>
#include <entt/entity/entity.hpp>
#include <vector>

/// One body cell of the slime snake. `at`/`to` are the lerp endpoints for the
/// current stride; `direction` is the facing used for that step.
struct SlimeSegment
{
    de::Vector2D<float> at{};
    de::Vector2D<float> to{};
    de::Vector2D<float> direction{1.0f, 0.0f};
};

/// Snake state on the head-owning entity. Segment sprite entities are synced
/// by SnakeSystem from this list.
struct SnakeComponent
{
    std::vector<SlimeSegment> segments;
    std::vector<entt::entity> segmentEntities;
    de::Vector2D<float> nextDirection{1.0f, 0.0f};
    float stride = 20.0f;
    float movementTimer = 0.0f;
    float movementProgress = 0.0f;
    int pendingGrowth = 0;

    static constexpr float movementInterval = 0.2f;
    static constexpr int scorePerBat = 100;
};

#endif // GAME_COMPONENTS_SNAKE_COMPONENT_HPP

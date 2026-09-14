#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/follow_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/camera2d.hpp>
#include <engine/systems/camera_system.hpp>

namespace de
{
void CameraSystem::run(entt::registry& registry)
{
    auto& dt = registry.ctx().get<DeltaTime>().value;
    auto view =
        registry.view<CameraComponent, FollowComponent, CameraBoundsComponent,
                      TransformComponent, DimensionComponent>();
    for (auto entity : view)
    {
        auto& camera = view.get<CameraComponent>(entity);
        auto& follow = view.get<FollowComponent>(entity);
        auto& bounds = view.get<CameraBoundsComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        auto& dimension = view.get<DimensionComponent>(entity);

        const auto* targetTransform =
            registry.valid(follow.target)
                ? registry.try_get<TransformComponent>(follow.target)
                : nullptr;
        if (targetTransform == nullptr)
        {
            continue;
        }

        // Predict where the target is heading. A target that does not move
        // by velocity is simply not extrapolated.
        Vector2D<float> predictedPosition = targetTransform->position;
        Vector2D<float> velocity;
        if (const auto* targetVelocity =
                registry.try_get<VelocityComponent>(follow.target))
        {
            velocity = targetVelocity->velocity;
        }

        // Extrapolate along the current velocity
        predictedPosition.x =
            predictedPosition.x + velocity.x * camera.predictionFactor * dt;
        predictedPosition.y =
            predictedPosition.y + velocity.y * camera.predictionFactor * dt;

        // Lerp towards the predicted position
        transform.position.x = transform.position.x +
                               (predictedPosition.x - transform.position.x) *
                                   camera.followSpeed * dt;
        transform.position.y = transform.position.y +
                               (predictedPosition.y - transform.position.y) *
                                   camera.followSpeed * dt;

        // Clamp the camera to the level bounds. The visible half-extents
        // come from Camera2D so this cannot drift from what is drawn.
        Camera2D view;
        view.zoom = camera.zoomLevel;
        view.viewWidth = dimension.width;
        view.viewHeight = dimension.height;
        const Vector2D<float> half = view.halfExtents();

        transform.position.x = std::max(
            half.x, std::min(transform.position.x,
                             static_cast<float>(bounds.levelWidth) - half.x));
        transform.position.y = std::max(
            half.y, std::min(transform.position.y,
                             static_cast<float>(bounds.levelHeight) - half.y));
    }
}

} // namespace de

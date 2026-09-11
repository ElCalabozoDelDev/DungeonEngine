#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/follow_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/vector_2d.hpp>
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
        auto& targetTransform = registry.get<TransformComponent>(follow.target);
        auto& targetVelocity = registry.get<VelocityComponent>(follow.target);
        auto& dimension = view.get<DimensionComponent>(entity);

        // Predict where the target is heading
        Vector2D<float> predictedPosition = targetTransform.position;
        Vector2D<float> velocity = targetVelocity.velocity;

        // Extrapolate along the current velocity
        predictedPosition.setX(predictedPosition.getX() +
                               velocity.getX() * camera.predictionFactor * dt);
        predictedPosition.setY(predictedPosition.getY() +
                               velocity.getY() * camera.predictionFactor * dt);

        // Lerp towards the predicted position
        transform.position.setX(
            transform.position.getX() +
            (predictedPosition.getX() - transform.position.getX()) *
                camera.followSpeed * dt);
        transform.position.setY(
            transform.position.getY() +
            (predictedPosition.getY() - transform.position.getY()) *
                camera.followSpeed * dt);

        // Clamp the camera to the level bounds
        float halfScreenWidth = (dimension.width / 2.0f) / camera.zoomLevel;
        float halfScreenHeight = (dimension.height / 2.0f) / camera.zoomLevel;
        transform.position.setX(std::max(
            halfScreenWidth,
            std::min(transform.position.getX(),
                     static_cast<float>(bounds.levelWidth - halfScreenWidth))));
        transform.position.setY(std::max(
            halfScreenHeight, std::min(transform.position.getY(),
                                       static_cast<float>(bounds.levelHeight -
                                                          halfScreenHeight))));
    }
}

} // namespace de

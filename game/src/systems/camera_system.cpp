#include "systems/camera_system.hpp"
#include "components/follow_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/transform_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "core/vector_2d.hpp"

void CameraSystem::run(entt::registry& registry) {
    auto view = registry.view<CameraComponent, FollowComponent, CameraBoundsComponent, TransformComponent>();
    auto &dt = registry.ctx().get<DeltaTime>().value;

    for (auto entity : view) {
        auto& camera = view.get<CameraComponent>(entity);
        auto& follow = view.get<FollowComponent>(entity);
        auto& bounds = view.get<CameraBoundsComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        auto& targetTransform = registry.get<TransformComponent>(follow.target);
        auto& targetVelocity = registry.get<VelocityComponent>(follow.target);

        // Predecir la posición del jugador
        Vector2D predictedPosition = targetTransform.position;
        Vector2D velocity = targetVelocity.velocity;

        // Predicción de la posición futura
        predictedPosition.m_x += velocity.m_x * camera.predictionFactor * dt;
        predictedPosition.m_y += velocity.m_y * camera.predictionFactor * dt;

        // Aplicar LERP hacia la posición predicha
        transform.position.m_x += (predictedPosition.m_x - transform.position.m_x) * camera.followSpeed * dt;
        transform.position.m_y += (predictedPosition.m_y - transform.position.m_y) * camera.followSpeed * dt;

        // Limitar la cámara a los límites del nivel
        float halfScreenWidth = camera.cameraWidth / 2.0f;
        float halfScreenHeight = camera.cameraHeight / 2.0f;
        transform.position.m_x = std::max(halfScreenWidth, std::min(transform.position.m_x, static_cast<float>(bounds.levelWidth - halfScreenWidth)));
        transform.position.m_y = std::max(halfScreenHeight, std::min(transform.position.m_y, static_cast<float>(bounds.levelHeight - halfScreenHeight)));
    }
}







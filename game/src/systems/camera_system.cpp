#include "systems/camera_system.hpp"
#include "components/follow_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/position_component.hpp"
#include "core/delta_time.hpp"
#include "core/vector_2d.hpp"
#include "loaders/config.hpp"

void CameraSystem::run(entt::registry& registry) {
    auto view = registry.view<CameraComponent, FollowComponent, CameraBoundsComponent, PositionComponent>();
    auto &dt = registry.ctx().get<DeltaTime>().value;
    for (auto entity : view) {
        auto& camera = view.get<CameraComponent>(entity);
        auto& follow = view.get<FollowComponent>(entity);
        auto& bounds = view.get<CameraBoundsComponent>(entity);
        auto& position = view.get<PositionComponent>(entity);

        Vector2D targetPosition = registry.get<PositionComponent>(follow.target).position;

        // Aplicamos LERP para un seguimiento suave
        position.position.m_x += (targetPosition.m_x - position.position.m_x) * camera.followSpeed * dt;
        position.position.m_y += (targetPosition.m_y - position.position.m_y) * camera.followSpeed * dt;

        // Obtenemos la configuración de la pantalla
        auto config = registry.ctx().get<Config>();
        float screenWidth = config.screenWidth;
        float screenHeight = config.screenHeight;

        // Aplicamos los límites de la cámara

        position.position.m_x = std::max(screenWidth / 2.0f, std::min(position.position.m_x, static_cast<float>(bounds.levelWidth - screenWidth / 2.0f)));
        position.position.m_y = std::max(screenHeight / 2.0f, std::min(position.position.m_y, static_cast<float>(bounds.levelHeight - screenHeight / 2.0f)));

    }
}


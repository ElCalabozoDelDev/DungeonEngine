#include "systems/camera_system.hpp"
#include "components/follow_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/transform_component.hpp"
#include "core/delta_time.hpp"
#include "core/vector_2d.hpp"
#include "loaders/config.hpp"

void CameraSystem::run(entt::registry& registry) {
    auto view = registry.view<CameraComponent, FollowComponent, CameraBoundsComponent, TransformComponent>();
    auto &dt = registry.ctx().get<DeltaTime>().value;

    for (auto entity : view) {
        auto& camera = view.get<CameraComponent>(entity);
        auto& follow = view.get<FollowComponent>(entity);
        auto& bounds = view.get<CameraBoundsComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);

        // Obtenemos la posición del jugador
        Vector2D targetPosition = registry.get<TransformComponent>(follow.target).position;

        // Aplicamos LERP para un seguimiento suave
        transform.position.m_x += (targetPosition.m_x - transform.position.m_x) * camera.followSpeed * dt;
        transform.position.m_y += (targetPosition.m_y - transform.position.m_y) * camera.followSpeed * dt;

        // Obtenemos la configuración de la pantalla
        auto config = registry.ctx().get<Config>();
        float screenWidth = config.screenWidth;
        float screenHeight = config.screenHeight;

        // Ajustar los límites en el eje X y Y teniendo en cuenta la mitad del tamaño de la pantalla
        float halfScreenWidth = screenWidth / 2.0f;
        float halfScreenHeight = screenHeight / 2.0f;

        transform.position.m_x = std::max(halfScreenWidth, std::min(transform.position.m_x, static_cast<float>(bounds.levelWidth - halfScreenWidth)));
        transform.position.m_y = std::max(halfScreenHeight, std::min(transform.position.m_y, static_cast<float>(bounds.levelHeight - halfScreenHeight)));

    }
}


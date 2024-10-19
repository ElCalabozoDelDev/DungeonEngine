#include "systems/camera_system.hpp"
#include "components/follow_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/transform_component.hpp"
#include "components/velocity_component.hpp"
#include "core/delta_time.hpp"
#include "core/vector_2d.hpp"
#include <components/dimension_component.hpp>

void CameraSystem::run(entt::registry& registry) {
	auto& dt = registry.ctx().get<DeltaTime>().value;
	auto view = registry.view<CameraComponent, FollowComponent, CameraBoundsComponent, TransformComponent, DimensionComponent>();
	for (auto entity : view) {
		auto& camera = view.get<CameraComponent>(entity);
		auto& follow = view.get<FollowComponent>(entity);
		auto& bounds = view.get<CameraBoundsComponent>(entity);
		auto& transform = view.get<TransformComponent>(entity);
		auto& targetTransform = registry.get<TransformComponent>(follow.target);
		auto& targetVelocity = registry.get<VelocityComponent>(follow.target);
		auto& dimension = view.get<DimensionComponent>(entity);

		// Predecir la posición del jugador
		Vector2D<float> predictedPosition = targetTransform.position;
		Vector2D<float> velocity = targetVelocity.velocity;

		// Predicción de la posición futura
		predictedPosition.setX(predictedPosition.getX() + velocity.getX() * camera.predictionFactor * dt);
		predictedPosition.setY(predictedPosition.getY() + velocity.getY() * camera.predictionFactor * dt);

		// Aplicar LERP hacia la posición predicha
		transform.position.setX(transform.position.getX() + (predictedPosition.getX() - transform.position.getX()) * camera.followSpeed * dt);
		transform.position.setY(transform.position.getY() + (predictedPosition.getY() - transform.position.getY()) * camera.followSpeed * dt);

		// Limitar la cámara a los límites del nivel
		float halfScreenWidth = (dimension.width / 2.0f) / camera.zoomLevel;
		float halfScreenHeight = (dimension.height / 2.0f) / camera.zoomLevel;
		transform.position.setX(std::max(halfScreenWidth, std::min(transform.position.getX(), static_cast<float>(bounds.levelWidth - halfScreenWidth))));
		transform.position.setY(std::max(halfScreenHeight, std::min(transform.position.getY(), static_cast<float>(bounds.levelHeight - halfScreenHeight))));
	}
}







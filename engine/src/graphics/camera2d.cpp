#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/camera2d.hpp>

namespace de
{
std::optional<Camera2D> activeCamera(entt::registry& registry)
{
    auto view =
        registry
            .view<CameraComponent, TransformComponent, DimensionComponent>();
    if (view.begin() == view.end())
    {
        return std::nullopt;
    }

    const auto entity = *view.begin();
    const auto& camera = view.get<CameraComponent>(entity);
    const auto& transform = view.get<TransformComponent>(entity);
    const auto& dimension = view.get<DimensionComponent>(entity);

    Camera2D result;
    result.position = transform.position;
    result.zoom = camera.zoomLevel;
    result.viewWidth = dimension.width;
    result.viewHeight = dimension.height;
    return result;
}

} // namespace de

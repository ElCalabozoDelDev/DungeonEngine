#include "imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/render.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/spatial/quadtree.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/widgets/gui.hpp>

namespace de
{
void RenderSystem::run(entt::registry& registry)
{
    SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();

    renderGraphics(registry);
    renderGUI(registry);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void RenderSystem::renderGUI(entt::registry& registry)
{
    SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
    registry.view<std::unique_ptr<gui::WidgetComponent>>().each(
        [&registry](auto entity, auto& widget_component)
        {
            widget_component->frame_begin();
            widget_component->frame_update(registry);
            widget_component->frame_end();
        });
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}
void RenderSystem::renderGraphics(entt::registry& registry)
{
    const auto& view = registry.view<CameraComponent, TransformComponent>();
    if (view.begin() == view.end())
    {
        // No camera yet (a scene that has not loaded, or one without one):
        // dereferencing begin() here was undefined behaviour.
        return;
    }
    auto cameraEntity = *view.begin();
    auto& camera = view.get<CameraComponent>(cameraEntity);
    auto& cameraPos = view.get<TransformComponent>(cameraEntity).position;
    auto& dimension = registry.get<DimensionComponent>(cameraEntity);

    // Zoom level
    float zoomLevel = camera.zoomLevel;

    // Visible extents at this zoom
    float adjustedCameraWidth = (dimension.width / zoomLevel);
    float adjustedCameraHeight = (dimension.height / zoomLevel);

    // Half extents, used to centre the view
    float halfAdjustedWidth = adjustedCameraWidth / 2.0f;
    float halfAdjustedHeight = adjustedCameraHeight / 2.0f;
    int margin = 32;
    // Visible area of the camera
    Box<float> cameraView{
        (cameraPos.getX() - halfAdjustedWidth),  // camera centre X
        (cameraPos.getY() - halfAdjustedHeight), // camera centre Y
        adjustedCameraWidth,                     // zoomed width
        adjustedCameraHeight                     // zoomed height
    };

    registry.view<std::shared_ptr<Render>>().each(
        [&registry, &cameraView, camera](entt::entity entity,
                                         std::shared_ptr<Render>& render)
        {
            if (render)
            {
                render->draw(registry, cameraView, camera.viewportOffsetX,
                             camera.viewportOffsetY, camera.zoomLevel);
            }
        });
}

} // namespace de

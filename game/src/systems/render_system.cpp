#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "graphics/render.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "widgets/gui.hpp"
#include "components/camera_component.hpp"
#include <components/dimension_component.hpp>
void RenderSystem::run(entt::registry& registry) {
	SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();

	renderGraphics(registry);
	renderGUI(registry);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void RenderSystem::renderGUI(entt::registry& registry) {
	SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
	registry.view<std::unique_ptr<gui::WidgetComponent>>().each(
		[&registry](auto entity, auto& widget_component) {
			widget_component->frame_begin();
			widget_component->frame_update(registry);
			widget_component->frame_end();
		});
	ImGui::Render();
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}
void RenderSystem::renderGraphics(entt::registry& registry) {
	const auto& view = registry.view<CameraComponent, TransformComponent>();
	auto cameraEntity = *view.begin();
	auto& camera = view.get<CameraComponent>(cameraEntity);
	auto& cameraPos = view.get<TransformComponent>(cameraEntity).position;
	auto& dimension = registry.get<DimensionComponent>(cameraEntity);

	// Obtener el nivel de zoom
	float zoomLevel = camera.zoomLevel;

	// Calcular la posición ajustada por el zoom
	int adjustedCameraWidth = static_cast<int>(dimension.width / zoomLevel);
	int adjustedCameraHeight = static_cast<int>(dimension.height / zoomLevel);

	// Obtener área de cámara visible
	AABB cameraView{
		static_cast<int>(cameraPos.getX() - adjustedCameraWidth / 2.0f),  // Ajustar el centro de la cámara
		static_cast<int>(cameraPos.getY() - adjustedCameraHeight / 2.0f), // Ajustar el centro de la cámara
		adjustedCameraWidth,  // Ajustar el ancho con zoom
		adjustedCameraHeight  // Ajustar el alto con zoom
	};




	registry.view<std::shared_ptr<Render>>().each(
		[&registry, &cameraView, camera](entt::entity entity,
			std::shared_ptr<Render>& render) {
				if (render) {
					render->draw(registry, cameraView, camera.viewportOffsetX, camera.viewportOffsetY);
				}
		});
}
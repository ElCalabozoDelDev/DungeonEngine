#include "systems/render_system.hpp"
#include "core/quadtree.hpp"
#include "graphics/render.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "widgets/gui.hpp"
#include "components/camera_component.hpp"
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

	// Obtener área de cámara visible
	AABB cameraView{
		static_cast<int>(cameraPos.getX() - camera.cameraWidth / 2.0f), // Centrar la cámara en X
		static_cast<int>(cameraPos.getY() - camera.cameraHeight / 2.0f), // Centrar la cámara en Y
		camera.cameraWidth, camera.cameraHeight 
	};
	registry.view<std::shared_ptr<Render>>().each(
		[&registry, &cameraView, camera](entt::entity entity,
			std::shared_ptr<Render>& render) {
				if (render) {
					render->draw(registry, cameraView, camera.viewportOffsetX, camera.viewportOffsetY);
				}
		});
}
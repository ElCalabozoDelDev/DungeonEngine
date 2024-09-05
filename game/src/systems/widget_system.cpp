#include "systems/widget_system.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "widgets/gui.hpp"
void WidgetSystem::run(entt::registry &registry) {
    // Obtener el renderer de SDL
    SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
    registry.view<std::unique_ptr<gui::WidgetComponent>>().each([&registry](auto entity, auto& widget_component) {
        widget_component->frame_begin();
        widget_component->frame_update(registry);
        widget_component->frame_end();
    });
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void WidgetSystem::handle(SDL_Event &event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}
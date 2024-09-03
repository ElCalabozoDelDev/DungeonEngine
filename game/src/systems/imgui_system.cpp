#include "systems/imgui_system.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"

void ImGuiSystem::run(entt::registry& registry) {
    // Obtener el renderer de SDL
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    // Crear una ventana de ImGui para mostrar la información del sistema
    auto view = registry.view<AnimationComponent, TextureComponent>();
    for (auto entity : view) {
        auto& animation = view.get<AnimationComponent>(entity);
        auto& texture = registry.get<TextureComponent>(entity);

        ImGui::Begin("Estado de la textura");
        ImGui::Text("spriteRow: %d", texture.spriteRow);
        ImGui::Text("spriteCol: %d", texture.spriteCol);
        ImGui::Text("currentSprite: %d", animation.currentSprite);
        ImGui::Text("currentFrame: %d", animation.currentFrame);
        ImGui::Text("totalFrames: %d", animation.totalFrames);
        ImGui::Text("animationTime: %.2f", animation.animationTime);
        ImGui::Text("timeSinceLastFrame: %.2f", animation.timeSinceLastFrame);
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void ImGuiSystem::handle(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}
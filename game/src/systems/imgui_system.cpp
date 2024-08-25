#include "systems/imgui_system.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"

void ImGuiSystem::init(SDL_Window* window, SDL_Renderer* renderer) {
    // Inicialización de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    // Configuración para SDL2
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

void ImGuiSystem::handleEvents(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void ImGuiSystem::newFrame() {
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSystem::render(entt::registry& registry, SDL_Renderer *renderer) {
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

    // Renderizar la interfaz de ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void ImGuiSystem::shutdown() {
    // Limpieza de ImGui
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
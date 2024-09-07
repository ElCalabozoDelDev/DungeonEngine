#include "plugins/imgui_plugin.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include "SDL_events.h"
#include "systems/widget_system.hpp"

void ImGuiPlugin::mount(GameLoop &gameLoop)
{
    auto imGuiSystem = std::make_shared<WidgetSystem>();
    gameLoop.addSetupCallback([imGuiSystem](entt::registry &registry)
    {
        registry.ctx().emplace<std::shared_ptr<WidgetSystem>>(imGuiSystem);
        SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
        SDL_Window *window = registry.ctx().get<SDL_Window *>();
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
    });

    gameLoop.addFrameBeginCallback([](entt::registry &registry)
    {
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();
    });

    gameLoop.addTeardownCallback([](entt::registry &registry)
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    });
    gameLoop.addSystem(imGuiSystem);
}
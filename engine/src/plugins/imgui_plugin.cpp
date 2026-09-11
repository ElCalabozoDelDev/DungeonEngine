#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/plugins/imgui_plugin.hpp>

namespace de
{
void ImGuiPlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [](entt::registry& registry)
        {
            if (registry.ctx().contains<StartupError>())
            {
                return;
            }
            SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
            SDL_Window* window = registry.ctx().get<Window>().get();
            // ImGui initialisation
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |=
                ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |=
                ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
            ImGui::StyleColorsDark();

            // SDL2 backend setup
            ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer2_Init(renderer);
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            ImGui_ImplSDL2_NewFrame();
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui::NewFrame();
        });

    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            if (ImGui::GetCurrentContext() == nullptr)
            {
                return;
            }
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        });
}

} // namespace de

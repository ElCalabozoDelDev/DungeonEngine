#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include <engine/graphics/logical_size.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/loaders/config.hpp>
#include <engine/plugins/imgui_plugin.hpp>

namespace de
{
void ImGuiPlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [](entt::registry& registry)
        {
            SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
            SDL_Window* window = registry.ctx().get<Window>().get();
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            // Keyboard and gamepad navigation are deliberately NOT enabled.
            // With them on, ImGui reports WantCaptureKeyboard for as long as
            // any window is focused; gameplay input honours that flag, so the
            // player would freeze for as long as the inspector is open.
            ImGui::StyleColorsDark();

            // Pixel-font path: nearest atlas sampling needs no baked AA lines.
            io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
            ImGuiStyle& style = ImGui::GetStyle();
            style.AntiAliasedLinesUseTex = false;
            style.AntiAliasedLines = false;
            style.AntiAliasedFill = false;

            ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer2_Init(renderer);
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            ImGui_ImplSDL2_NewFrame();
            ImGui_ImplSDLRenderer2_NewFrame();

            // Widgets author in SDL logical space (320×180). Force ImGui to
            // match so hit-tests and DisplaySize agree with draw coords.
            ImGuiIO& io = ImGui::GetIO();
            const auto [lw, lh] = logicalSize(registry);
            float mx = 0.0f;
            float my = 0.0f;
            if (auto* renderer = registry.ctx().find<MainRenderer>();
                renderer != nullptr && renderer->get() != nullptr)
            {
                SDL_RenderWindowToLogical(
                    renderer->get(), static_cast<int>(io.MousePos.x),
                    static_cast<int>(io.MousePos.y), &mx, &my);
            }
            else
            {
                const float scale = windowToLogicalScale(registry);
                mx = io.MousePos.x / scale;
                my = io.MousePos.y / scale;
            }
            io.DisplaySize = ImVec2(lw, lh);
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
            if (io.MousePos.x >= 0.0f && io.MousePos.y >= 0.0f)
            {
                io.MousePos = ImVec2(mx, my);
            }

            ImGui::NewFrame();
        });

    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            (void)registry;
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

#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include <engine/core/asset_paths.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/ui_fonts.hpp>
#include <engine/plugins/imgui_plugin.hpp>
#include <filesystem>

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

            UiFonts fonts{};
            if (const auto* assets = registry.ctx().find<AssetPaths>();
                assets != nullptr)
            {
                const auto fontPath = assets->resolve("fonts/04B_30.ttf");
                if (std::filesystem::exists(fontPath))
                {
                    ImFontConfig cfg;
                    cfg.PixelSnapH = true;
                    cfg.OversampleH = 1;
                    cfg.OversampleV = 1;
                    // Debug / fallback only — title and game UI use BMFont.
                    fonts.body = io.Fonts->AddFontFromFileTTF(
                        fontPath.string().c_str(), 16.0f, &cfg);
                }
            }
            registry.ctx().emplace<UiFonts>(fonts);

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

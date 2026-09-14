#include <SDL.h>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/plugins/input_plugin.hpp>
#include <imgui.h>
#include <imgui/imgui_impl_sdl2.h>

namespace de
{
void InputPlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [](entt::registry& registry)
        {
            registry.ctx().emplace<InputState>();
            registry.ctx().emplace<ActionMap>();
        });

    gameLoop.addFrameBeginCallback(
        [](entt::registry& registry)
        {
            auto& input = registry.ctx().get<InputState>();
            auto& controlFlow = registry.ctx().get<ControlFlow>();

            input.beginFrame();

            SDL_Event event;
            while (SDL_PollEvent(&event) != 0)
            {
                // Forwarded before ImGui::NewFrame, which is where ImGui
                // wants them. The game used to poll after NewFrame, so every
                // ImGui interaction was applied a frame late.
                ImGui_ImplSDL2_ProcessEvent(&event);

                if (event.type == SDL_QUIT)
                {
                    controlFlow = ControlFlow::Exit;
                }
                else if (event.type == SDL_WINDOWEVENT &&
                         event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    controlFlow = ControlFlow::Exit;
                }
            }

            int keyCount = 0;
            const Uint8* keys = SDL_GetKeyboardState(&keyCount);
            input.setKeyboard(keys, keyCount);

            int mouseX = 0;
            int mouseY = 0;
            const Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
            input.setMouse(mouseX, mouseY, buttons);

            // Typing into a GUI field must not also drive the player. The
            // flags describe the frame ImGui just finished, which is exactly
            // what is wanted here: NewFrame has not run yet.
            if (ImGui::GetCurrentContext() != nullptr)
            {
                const ImGuiIO& io = ImGui::GetIO();
                input.setCaptured(io.WantCaptureKeyboard, io.WantCaptureMouse);
            }
        });
}

} // namespace de

#include <engine/core/game_loop.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/menu_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

void MenuWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));

    ImGui::Begin("DungeonEngine", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse);

    const auto* state = registry.ctx().find<GameState>();
    if (state != nullptr && state->gameOver)
    {
        ImGui::TextUnformatted("You died.");
        ImGui::Text("Items collected: %d / %d", state->itemsCollected,
                    state->itemsTotal);
        ImGui::Separator();
    }
    else
    {
        ImGui::TextUnformatted("A C++23 + SDL2 boilerplate.");
        ImGui::TextUnformatted("Arrows or WASD to move. Esc pauses.");
        ImGui::Separator();
    }

    // Keyboard as well as mouse. ImGui's own keyboard navigation is
    // deliberately disabled (it would hold the keyboard away from gameplay),
    // so the shortcuts are read from the action map instead.
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const bool confirmPressed = input != nullptr && actions != nullptr &&
                                actions->wasPressed(*input, "confirm");
    const bool quitPressed = input != nullptr && actions != nullptr &&
                             actions->wasPressed(*input, "pause");

    if (ImGui::Button("Play  (Enter)", ImVec2(160, 0)) || confirmPressed)
    {
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<InGameScene>());
    }
    if (ImGui::Button("Quit  (Esc)", ImVec2(160, 0)) || quitPressed)
    {
        registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
    }

    ImGui::End();
}

#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/components/health_component.hpp>
#include <game/components/player_component.hpp>
#include <game/scene/menu_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/hud_widget.hpp>
#include <imgui.h>
#include <memory>
#include <string>

using namespace de;

void HudWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const auto* state = registry.ctx().find<GameState>();

    // --- HUD ---
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("HUD", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    auto players = registry.view<PlayerComponent, HealthComponent>();
    if (players.begin() != players.end())
    {
        const auto& health = players.get<HealthComponent>(*players.begin());

        std::string hearts;
        for (int i = 0; i < health.max; ++i)
        {
            hearts += i < health.current ? "[#]" : "[ ]";
        }
        ImGui::TextUnformatted(hearts.c_str());
    }

    if (state != nullptr)
    {
        ImGui::Text("Items %d / %d", state->itemsCollected, state->itemsTotal);
    }
    ImGui::End();

    // --- Pause overlay ---
    const auto* paused = registry.ctx().find<Paused>();
    if (paused == nullptr || !paused->value)
    {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::Begin("Paused", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::TextUnformatted("Paused. Esc resumes.");
    ImGui::Separator();
    if (ImGui::Button("Back to menu", ImVec2(160, 0)))
    {
        registry.ctx().get<Paused>().value = false;
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<MenuScene>());
    }
    ImGui::End();
}

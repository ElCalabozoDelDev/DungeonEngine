#include <SDL.h>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/options_scene.hpp>
#include <game/widgets/title_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

void TitleWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Tiled background pattern when the texture is available.
    if (auto* textures = registry.ctx().find<TextureCache>();
        textures != nullptr)
    {
        if (SDL_Texture* pattern = textures->get("bg-pattern");
            pattern != nullptr)
        {
            int tw = 0;
            int th = 0;
            SDL_QueryTexture(pattern, nullptr, nullptr, &tw, &th);
            if (tw > 0 && th > 0)
            {
                ImDrawList* draw = ImGui::GetBackgroundDrawList();
                const ImTextureID id = reinterpret_cast<ImTextureID>(pattern);
                for (float y = viewport->Pos.y;
                     y < viewport->Pos.y + viewport->Size.y; y += th)
                {
                    for (float x = viewport->Pos.x;
                         x < viewport->Pos.x + viewport->Size.x; x += tw)
                    {
                        draw->AddImage(id, ImVec2(x, y),
                                       ImVec2(x + tw, y + th));
                    }
                }
            }
        }
    }

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::Begin("title", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoBackground);

    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextUnformatted("DUNGEON SLIME");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    const bool confirmPressed = input != nullptr && actions != nullptr &&
                                actions->wasPressed(*input, "confirm");

    auto playUi = [&]()
    {
        if (auto* audio = registry.ctx().find<AudioManager>())
        {
            audio->playSound("ui");
        }
    };

    if (ImGui::Button("START", ImVec2(160, 0)) || confirmPressed)
    {
        playUi();
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<InGameScene>());
    }
    if (ImGui::Button("OPTIONS", ImVec2(160, 0)))
    {
        playUi();
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<OptionsScene>());
    }
    if (ImGui::Button("QUIT", ImVec2(160, 0)))
    {
        playUi();
        registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
    }

    ImGui::End();
}

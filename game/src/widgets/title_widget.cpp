#include <SDL.h>
#include <engine/audio/audio_manager.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/loaders/config.hpp>
#include <engine/scene/scene_system.hpp>
#include <game/scene/in_game_scene.hpp>
#include <game/scene/options_scene.hpp>
#include <game/widgets/title_widget.hpp>
#include <imgui.h>
#include <memory>

using namespace de;

namespace
{
ImVec2 logicalSize(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>(); config != nullptr)
    {
        return ImVec2(config->cameraWidth, config->cameraHeight);
    }
    return ImVec2(320.0f, 180.0f);
}

void playUi(entt::registry& registry)
{
    if (auto* audio = registry.ctx().find<AudioManager>())
    {
        audio->playSound("ui");
    }
}

bool menuButton(const char* label, bool selected, const ImVec2& size)
{
    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.55f, 0.65f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.65f, 0.75f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.45f, 0.55f, 0.85f, 1.0f));
    }
    const bool clicked = ImGui::Button(label, size);
    if (selected)
    {
        ImGui::PopStyleColor(3);
    }
    return clicked;
}
} // namespace

void TitleWidget::render(entt::registry& registry, de::gui::Hooks& h)
{
    const ImVec2 screen = logicalSize(registry);
    auto [focus, setFocus] = h.use_state(0); // START / OPTIONS / QUIT

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
                for (float y = 0.0f; y < screen.y; y += static_cast<float>(th))
                {
                    for (float x = 0.0f; x < screen.x;
                         x += static_cast<float>(tw))
                    {
                        draw->AddImage(id, ImVec2(x, y),
                                       ImVec2(x + tw, y + th));
                    }
                }
            }
        }
    }

    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    if (input != nullptr && actions != nullptr)
    {
        if (actions->wasPressedRaw(*input, "move_up") ||
            actions->wasPressedRaw(*input, "move_left"))
        {
            setFocus((focus + 2) % 3);
            playUi(registry);
        }
        if (actions->wasPressedRaw(*input, "move_down") ||
            actions->wasPressedRaw(*input, "move_right"))
        {
            setFocus((focus + 1) % 3);
            playUi(registry);
        }
    }

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, screen.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("title", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoBackground);

    ImGui::SetWindowFontScale(0.9f);
    ImGui::TextUnformatted("DUNGEON SLIME");
    ImGui::SetWindowFontScale(0.55f);
    ImGui::Separator();

    const bool confirm = input != nullptr && actions != nullptr &&
                         actions->wasPressedRaw(*input, "confirm");

    if (menuButton("START", focus == 0, ImVec2(90, 0)) ||
        (confirm && focus == 0))
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<InGameScene>());
    }
    if (menuButton("OPTIONS", focus == 1, ImVec2(90, 0)) ||
        (confirm && focus == 1))
    {
        playUi(registry);
        registry.ctx().get<SceneSystem>().requestScene(
            std::make_unique<OptionsScene>());
    }
    if (menuButton("QUIT", focus == 2, ImVec2(90, 0)) ||
        (confirm && focus == 2))
    {
        playUi(registry);
        registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

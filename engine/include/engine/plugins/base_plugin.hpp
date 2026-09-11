#ifndef DE_PLUGINS_BASE_PLUGIN_HPP
#define DE_PLUGINS_BASE_PLUGIN_HPP

#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config.hpp>
#include <engine/plugins/imgui_plugin.hpp>
#include <engine/plugins/plugin.hpp>
#include <engine/plugins/sdl_plugin.hpp>
#include <engine/plugins/widget_plugin.hpp>
#include <entt/entt.hpp>
#include <string>

namespace de
{
/// The engine's default bundle: window and renderer, Dear ImGui, the widget
/// layer, and the systems every 2D game here needs (transform integration,
/// sprite animation, rendering).
///
/// Everything it mounts is engine-owned. Input handling and gameplay systems
/// belong to the game's own plugin.
class BasePlugin final : public Plugin
{
public:
    explicit BasePlugin(const std::string& configPath);

    void mount(GameLoop& gameLoop) override;

private:
    SDLPlugin m_sdl;
    ImGuiPlugin m_imgui;
    WidgetPlugin m_widget;
    Config m_config;
};

} // namespace de

#endif // DE_PLUGINS_BASE_PLUGIN_HPP

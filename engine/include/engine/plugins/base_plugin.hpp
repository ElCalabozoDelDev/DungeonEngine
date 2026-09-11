#ifndef DE_PLUGINS_BASE_PLUGIN_HPP
#define DE_PLUGINS_BASE_PLUGIN_HPP

#include <SDL.h>
#include <engine/core/asset_paths.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config.hpp>
#include <engine/plugins/imgui_plugin.hpp>
#include <engine/plugins/plugin.hpp>
#include <engine/plugins/sdl_plugin.hpp>
#include <engine/plugins/widget_plugin.hpp>
#include <entt/entt.hpp>
#include <utility>

namespace de
{
/// The engine's default bundle: window and renderer, Dear ImGui, the widget
/// layer, and the systems every 2D game here needs (transform integration,
/// sprite animation, rendering).
///
/// Takes an already-validated Config rather than a path to parse: loading can
/// fail, and a constructor cannot report that. main() loads the config and
/// exits with a message if it is bad, so this type cannot be built from one
/// that never parsed.
class BasePlugin final : public Plugin
{
public:
    BasePlugin(Config config, AssetPaths assets)
        : m_config(std::move(config)), m_assets(std::move(assets))
    {
    }

    void mount(GameLoop& gameLoop) override;

private:
    SDLPlugin m_sdl;
    ImGuiPlugin m_imgui;
    WidgetPlugin m_widget;
    Config m_config;
    AssetPaths m_assets;
};

} // namespace de

#endif // DE_PLUGINS_BASE_PLUGIN_HPP

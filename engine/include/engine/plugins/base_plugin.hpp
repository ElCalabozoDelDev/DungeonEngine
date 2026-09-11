#ifndef DE_PLUGINS_BASE_PLUGIN_HPP
#define DE_PLUGINS_BASE_PLUGIN_HPP

#include <engine/core/asset_paths.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/loaders/config.hpp>
#include <engine/plugins/plugin.hpp>
#include <utility>

namespace de
{
/// The engine's default bundle: window and renderer, input, Dear ImGui, the
/// widget layer, and the systems every 2D game here needs.
///
/// Takes an already-validated Config rather than a path to parse: loading can
/// fail, and a constructor cannot report that. main() loads the config and
/// exits with a message if it is bad.
///
/// The sub-plugins it needs are created in mount() and handed to the loop,
/// which owns them, instead of being members whose lifetime this type has to
/// guarantee.
class BasePlugin final : public Plugin
{
public:
    BasePlugin(Config config, AssetPaths assets)
        : m_config(std::move(config)), m_assets(std::move(assets))
    {
    }

    void mount(GameLoop& gameLoop) override;

private:
    Config m_config;
    AssetPaths m_assets;
};

} // namespace de

#endif // DE_PLUGINS_BASE_PLUGIN_HPP

#ifndef DE_PLUGINS_IMGUI_PLUGIN_HPP
#define DE_PLUGINS_IMGUI_PLUGIN_HPP
#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

namespace de
{
class ImGuiPlugin : public Plugin
{
public:
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_IMGUI_PLUGIN_HPP

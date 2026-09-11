#ifndef DE_PLUGINS_WIDGET_PLUGIN_HPP
#define DE_PLUGINS_WIDGET_PLUGIN_HPP
#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

namespace de
{
class WidgetPlugin : public Plugin
{
public:
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_WIDGET_PLUGIN_HPP

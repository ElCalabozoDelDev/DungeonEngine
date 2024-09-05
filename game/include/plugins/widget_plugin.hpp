#ifndef WIDGET_PLUGIN_HPP
#define WIDGET_PLUGIN_HPP

#include "plugins/plugin.hpp"
#include "core/game_loop.hpp"

class WidgetPlugin : public Plugin
{
public:
    void mount(GameLoop &gameLoop) override;
};
#endif
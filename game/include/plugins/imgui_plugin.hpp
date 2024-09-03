#ifndef IMGUI_PLUGIN_HPP
#define IMGUI_PLUGIN_HPP

#include "plugin.hpp"
#include "core/game_loop.hpp"

class ImGuiPlugin : public Plugin
{
public:
    void mount(GameLoop &gameLoop) override;
};

#endif
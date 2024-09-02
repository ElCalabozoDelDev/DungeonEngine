#ifndef GAME_PLUGIN_HPP
#define GAME_PLUGIN_HPP

#include "plugin.hpp"
#include "systems/scene_system.hpp"
#include "scene/in_game_scene.hpp"
#include "core/game_loop.hpp"

class GamePlugin : public Plugin
{
public:
    void mount(GameLoop &gameLoop) override;
};

#endif
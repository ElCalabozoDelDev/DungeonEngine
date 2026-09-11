#ifndef GAME_PLUGINS_GAME_PLUGIN_HPP
#define GAME_PLUGINS_GAME_PLUGIN_HPP

#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

/// Everything specific to this game: event pumping, input-driven movement,
/// the camera, the scene stack and the debug inspector.
class GamePlugin : public de::Plugin
{
public:
    void mount(de::GameLoop& gameLoop) override;
};

#endif // GAME_PLUGINS_GAME_PLUGIN_HPP

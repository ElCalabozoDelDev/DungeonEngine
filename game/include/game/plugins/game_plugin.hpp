#ifndef GAME_PLUGINS_GAME_PLUGIN_HPP
#define GAME_PLUGINS_GAME_PLUGIN_HPP

#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

/// Everything specific to this game: key bindings, shared state and assets,
/// the snake and bat systems, pause handling, and the first scene. Mount it
/// after BasePlugin, whose systems and context it relies on.
class GamePlugin : public de::Plugin
{
public:
    void mount(de::GameLoop& gameLoop) override;
};

#endif // GAME_PLUGINS_GAME_PLUGIN_HPP

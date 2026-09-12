#ifndef DE_PLUGINS_INPUT_PLUGIN_HPP
#define DE_PLUGINS_INPUT_PLUGIN_HPP

#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

namespace de
{
/// Pumps SDL events and publishes InputState and ActionMap.
///
/// This is the only place that talks to the SDL event queue: events are drained
/// once per frame, forwarded to Dear ImGui, and turned into an InputState
/// snapshot the rest of the frame reads. Previously the game pumped events
/// itself while MovementSystem separately called SDL_GetKeyboardState.
class InputPlugin : public Plugin
{
public:
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_INPUT_PLUGIN_HPP

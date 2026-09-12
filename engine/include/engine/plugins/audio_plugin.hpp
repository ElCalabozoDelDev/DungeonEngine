#ifndef DE_PLUGINS_AUDIO_PLUGIN_HPP
#define DE_PLUGINS_AUDIO_PLUGIN_HPP

#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

namespace de
{
/// Opens the mixer and publishes AudioManager in the registry context.
///
/// A failure to open the audio device is reported and then ignored: no sound
/// is a worse game, not a broken one, so it must not stop startup.
class AudioPlugin : public Plugin
{
public:
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_AUDIO_PLUGIN_HPP

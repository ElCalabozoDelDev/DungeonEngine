#include <engine/audio/audio_manager.hpp>
#include <engine/plugins/audio_plugin.hpp>
#include <iostream>
#include <string>

namespace de
{
void AudioPlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [](entt::registry& registry)
        {
            auto& audio = registry.ctx().emplace<AudioManager>();
            std::string error;
            if (!audio.open(&error))
            {
                std::cerr << "Audio: could not open the mixer (" << error
                          << "). Continuing without sound." << std::endl;
            }
        });

    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            // Before SDL_Quit, like the other SDL-owned resources.
            registry.ctx().erase<AudioManager>();
        });
}

} // namespace de

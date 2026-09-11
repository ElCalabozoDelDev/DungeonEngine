#ifndef DE_PLUGINS_SDL_PLUGIN_HPP
#define DE_PLUGINS_SDL_PLUGIN_HPP
#include <SDL.h>
#include <engine/core/game_loop.hpp>
#include <engine/plugins/plugin.hpp>

namespace de
{
class SDLPlugin : public Plugin
{
private:
    Uint32 m_frameStart, m_frameTime, m_frameDelay = 0;

public:
    SDLPlugin() {}
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_SDL_PLUGIN_HPP

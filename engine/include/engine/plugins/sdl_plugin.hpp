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
    Uint32 m_frameStart = 0;
    Uint32 m_frameTime = 0;
    Uint32 m_frameDelay = 0;
    bool m_vsync = true;

public:
    SDLPlugin() {}
    void mount(GameLoop& gameLoop) override;
};

} // namespace de

#endif // DE_PLUGINS_SDL_PLUGIN_HPP

#ifndef SDL_PLUGIN_HPP
#define SDL_PLUGIN_HPP

#include "plugin.hpp"
#include "core/game_loop.hpp"
#include <SDL.h>

class SDLPlugin : public Plugin
{
private:
    Uint32 m_frameStart, m_frameTime, m_frameDelay = 0;
public:
    SDLPlugin() {}
    void mount(GameLoop &gameLoop) override;
};

#endif
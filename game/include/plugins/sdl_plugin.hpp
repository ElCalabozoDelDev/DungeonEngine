#ifndef SDL_PLUGIN_HPP
#define SDL_PLUGIN_HPP

#include "plugin.hpp"
#include "core/game_loop.hpp"
#include <SDL.h>
#include "loaders/config.hpp"
class SDLPlugin : public Plugin
{
private:
    Config m_config;
    Uint32 m_frameStart, m_frameTime, m_frameDelay = 0;
public:
    SDLPlugin(const Config &config): m_config(config) {}
    void mount(GameLoop &gameLoop) override;
};

#endif
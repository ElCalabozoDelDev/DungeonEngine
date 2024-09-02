#ifndef SDL_PLUGIN_HPP
#define SDL_PLUGIN_HPP

#include "plugin.hpp"
#include "core/game_loop.hpp"
#include <SDL.h>
#include "core/config.hpp"
#include "scene/delta_time.hpp"
class SDLPlugin : public Plugin
{
private:
    Config m_config;
    Uint32 m_frameStart, m_frameTime, m_frameDelay = 0;
public:
    SDLPlugin(const Config &config): m_config(config) {}
    void mount(GameLoop &gameLoop) override
    {
        gameLoop.addSetupCallback([this](entt::registry &registry)
        {
            m_frameDelay = 1000 / m_config.frameRate;
            SDL_Init(SDL_INIT_EVERYTHING);
            int flags = m_config.fullScreen ? SDL_WINDOW_FULLSCREEN : 0;
            
            SDL_Window *window =
                SDL_CreateWindow(m_config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, m_config.screenWidth, m_config.screenHeight, flags);
            SDL_Renderer *renderer =
                SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            registry.ctx().emplace<DeltaTime *>(new DeltaTime {0.0f});
            registry.ctx().emplace<SDL_Window *>(window);
            registry.ctx().emplace<SDL_Renderer *>(renderer); 
        });

        gameLoop.addFrameBeginCallback([this](entt::registry &registry)
        {   
            m_frameStart = SDL_GetTicks();
            SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
            // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        });

        gameLoop.addFrameEndCallback([this](entt::registry &registry)
        {
            SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
            SDL_RenderPresent(renderer);
            m_frameTime = SDL_GetTicks() - m_frameStart;
            if (m_frameTime < m_frameDelay)
            {
                SDL_Delay(m_frameDelay - m_frameTime);
            }
        });

        gameLoop.addTeardownCallback([](entt::registry &registry)
        {
            SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
            SDL_Window *window = registry.ctx().get<SDL_Window *>();

            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit(); 
        });
    }
};

#endif
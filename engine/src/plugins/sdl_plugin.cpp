#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/config.hpp>
#include <engine/plugins/sdl_plugin.hpp>
#include <iostream>

namespace de
{
void SDLPlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [this](entt::registry& registry)
        {
            const auto& config = registry.ctx().get<Config>();
            m_frameDelay = 1000 / config.frameRate;
            SDL_Init(SDL_INIT_EVERYTHING);
            int flags = config.fullScreen ? SDL_WINDOW_FULLSCREEN : 0;

            WindowPtr window(
                SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, config.screenWidth,
                                 config.screenHeight, flags));
            RendererPtr renderer(SDL_CreateRenderer(
                window.get(), -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));

            SDL_Renderer* rawRenderer = renderer.get();

            registry.ctx().emplace<DeltaTime>(DeltaTime{0.0f});
            registry.ctx().emplace<Window>(Window{std::move(window)});
            registry.ctx().emplace<MainRenderer>(
                MainRenderer{std::move(renderer)});
            registry.ctx().emplace<TextureCache>(rawRenderer);
        });

    gameLoop.addFrameBeginCallback(
        [this](entt::registry& registry)
        {
            m_frameStart = SDL_GetTicks();
            SDL_RenderClear(registry.ctx().get<MainRenderer>().get());
        });

    gameLoop.addFrameEndCallback(
        [this](entt::registry& registry)
        {
            SDL_RenderPresent(registry.ctx().get<MainRenderer>().get());
            m_frameTime = SDL_GetTicks() - m_frameStart;
            if (m_frameTime < m_frameDelay)
            {
                SDL_Delay(m_frameDelay - m_frameTime);
            }
        });

    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            // Order matters and is the reason these are erased explicitly
            // rather than left to the registry: textures must go before the
            // renderer that owns them, the renderer before the window, and
            // all of it before SDL_Quit.
            registry.ctx().erase<TextureCache>();
            registry.ctx().erase<MainRenderer>();
            registry.ctx().erase<Window>();
            SDL_Quit();
        });
}

} // namespace de

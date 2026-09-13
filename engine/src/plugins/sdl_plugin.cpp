#include <engine/core/startup_error.hpp>
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

            if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            {
                registry.ctx().emplace<StartupError>(StartupError{
                    std::string("SDL_Init failed: ") + SDL_GetError()});
                return;
            }

            m_vsync = config.vsync;
            int flags = config.fullScreen ? SDL_WINDOW_FULLSCREEN : 0;
            // Nearest upscale for the 320→1280 logical present (pixel art).
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            WindowPtr window(
                SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, config.screenWidth,
                                 config.screenHeight, flags));
            if (!window)
            {
                registry.ctx().emplace<StartupError>(StartupError{
                    std::string("SDL_CreateWindow failed: ") + SDL_GetError()});
                return;
            }

            // Either vsync paces the frame or the manual limiter does --
            // never both. Enabling both meant SDL_Delay sleeping on top of a
            // present that had already blocked, which produced judder.
            Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
            if (m_vsync)
            {
                rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
            }
            RendererPtr renderer(
                SDL_CreateRenderer(window.get(), -1, rendererFlags));
            if (!renderer)
            {
                // No accelerated renderer (a headless CI machine, a remote
                // session, an old driver): software still draws.
                std::cerr << "SDL_CreateRenderer: no accelerated renderer ("
                          << SDL_GetError() << "), falling back to software."
                          << std::endl;
                rendererFlags = (rendererFlags & ~SDL_RENDERER_ACCELERATED) |
                                SDL_RENDERER_SOFTWARE;
                renderer.reset(
                    SDL_CreateRenderer(window.get(), -1, rendererFlags));
            }
            if (!renderer)
            {
                registry.ctx().emplace<StartupError>(
                    StartupError{std::string("SDL_CreateRenderer failed: ") +
                                 SDL_GetError()});
                return;
            }

            SDL_Renderer* rawRenderer = renderer.get();

            // Draw in the camera's coordinate space and let SDL scale it to
            // the window. Everything downstream works in these logical
            // pixels, so window size stops leaking into the draw maths.
            SDL_RenderSetLogicalSize(rawRenderer,
                                     static_cast<int>(config.logicalWidth),
                                     static_cast<int>(config.logicalHeight));

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
            if (!m_vsync)
            {
                m_frameTime = SDL_GetTicks() - m_frameStart;
                if (m_frameTime < m_frameDelay)
                {
                    SDL_Delay(m_frameDelay - m_frameTime);
                }
            }
        });

    gameLoop.addTeardownCallback(
        [](entt::registry& registry)
        {
            // Order matters and is the reason these are erased explicitly
            // rather than left to the registry: textures must go before the
            // renderer that owns them, the renderer before the window, and
            // all of it before SDL_Quit. Any of them may be absent if startup
            // failed part-way through.
            registry.ctx().erase<TextureCache>();
            registry.ctx().erase<MainRenderer>();
            registry.ctx().erase<Window>();
            SDL_Quit();
        });
}

} // namespace de

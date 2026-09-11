#ifndef DE_GRAPHICS_SDL_RESOURCES_HPP
#define DE_GRAPHICS_SDL_RESOURCES_HPP

#include <SDL.h>
#include <memory>

namespace de
{
struct WindowDeleter
{
    void operator()(SDL_Window* window) const noexcept
    {
        if (window != nullptr)
        {
            SDL_DestroyWindow(window);
        }
    }
};

struct RendererDeleter
{
    void operator()(SDL_Renderer* renderer) const noexcept
    {
        if (renderer != nullptr)
        {
            SDL_DestroyRenderer(renderer);
        }
    }
};

using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;

/// The game window, owned.
///
/// A named wrapper rather than a bare SDL_Window* because the registry context
/// is keyed by type: storing the raw pointer means there can only ever be one,
/// and nothing distinguishes it from any other SDL_Window* a game might want
/// to keep around.
struct Window
{
    WindowPtr handle;

    SDL_Window* get() const noexcept { return handle.get(); }
};

/// The renderer the game draws through, owned. Same reasoning as Window.
struct MainRenderer
{
    RendererPtr handle;

    SDL_Renderer* get() const noexcept { return handle.get(); }
};

} // namespace de

#endif // DE_GRAPHICS_SDL_RESOURCES_HPP

#ifndef DE_GRAPHICS_TEXTURE_CACHE_HPP
#define DE_GRAPHICS_TEXTURE_CACHE_HPP

#include <SDL.h>
#include <map>
#include <string>
#include <string_view>

namespace de
{
/// Owns the textures loaded for a level, keyed by id.
///
/// Lives in the registry context (`registry.ctx().get<TextureCache>()`), not
/// in a singleton: its textures belong to a specific SDL_Renderer and must be
/// destroyed before it, which a process-lifetime static cannot guarantee.
///
/// The destructor calls SDL_DestroyTexture on every entry. The previous
/// TextureManager only cleared its map, leaking every texture it ever loaded.
class TextureCache
{
public:
    /// Parameters for a single draw call.
    ///
    /// `source` is the region of the texture to sample; the destination is
    /// that region's size multiplied by `scale`, placed at `position`.
    struct DrawParams
    {
        SDL_Rect source{};
        SDL_Point position{};
        float scale = 1.0f;
        double angle = 0.0;
        Uint8 alpha = 255;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
    };

    explicit TextureCache(SDL_Renderer* renderer) noexcept
        : m_renderer(renderer)
    {
    }

    ~TextureCache();

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;
    TextureCache(TextureCache&& other) noexcept;
    TextureCache& operator=(TextureCache&& other) noexcept;

    /// Loads `fileName` and stores it under `id`. Returns false and leaves the
    /// cache untouched if the file cannot be read or the texture cannot be
    /// created; the reason goes to stderr.
    bool load(std::string_view id, const std::string& fileName);

    /// Returns nullptr when `id` was never loaded. Callers must not destroy it.
    SDL_Texture* get(std::string_view id) const;

    /// Draws `id`. A miss is a no-op rather than a crash: the old code used
    /// map::operator[], which inserted a null texture for every typo.
    void draw(std::string_view id, const DrawParams& params) const;

    void remove(std::string_view id);
    void clear();

    std::size_t size() const noexcept { return m_textures.size(); }

    /// Source rect for cell (row, column) of a uniform sprite sheet.
    static SDL_Rect frameRect(int width, int height, int row, int column)
    {
        return SDL_Rect{width * column, height * row, width, height};
    }

    /// Source rect for tile (row, column) of a tileset that carries an outer
    /// `margin` and `spacing` between tiles.
    static SDL_Rect tileRect(int width, int height, int row, int column,
                             int margin, int spacing)
    {
        return SDL_Rect{margin + (spacing + width) * column,
                        margin + (spacing + height) * row, width, height};
    }

private:
    void destroyAll() noexcept;

    SDL_Renderer* m_renderer = nullptr;
    std::map<std::string, SDL_Texture*, std::less<>> m_textures;
};

} // namespace de

#endif // DE_GRAPHICS_TEXTURE_CACHE_HPP

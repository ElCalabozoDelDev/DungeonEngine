#include <SDL_image.h>
#include <engine/graphics/texture_cache.hpp>
#include <iostream>
#include <utility>

namespace de
{
TextureCache::~TextureCache() { destroyAll(); }

TextureCache::TextureCache(TextureCache&& other) noexcept
    : m_renderer(other.m_renderer), m_textures(std::move(other.m_textures))
{
    other.m_textures.clear();
    other.m_renderer = nullptr;
}

TextureCache& TextureCache::operator=(TextureCache&& other) noexcept
{
    if (this != &other)
    {
        destroyAll();
        m_renderer = other.m_renderer;
        m_textures = std::move(other.m_textures);
        other.m_textures.clear();
        other.m_renderer = nullptr;
    }
    return *this;
}

void TextureCache::destroyAll() noexcept
{
    for (auto& [id, texture] : m_textures)
    {
        if (texture != nullptr)
        {
            SDL_DestroyTexture(texture);
        }
    }
    m_textures.clear();
}

bool TextureCache::load(std::string_view id, const std::string& fileName)
{
    SDL_Surface* surface = IMG_Load(fileName.c_str());
    if (surface == nullptr)
    {
        std::cerr << "TextureCache: could not load '" << fileName
                  << "': " << IMG_GetError() << '\n';
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        std::cerr << "TextureCache: could not create a texture from '"
                  << fileName << "': " << SDL_GetError() << '\n';
        return false;
    }

    // Replacing an id must not leak the texture it used to hold.
    auto key = std::string(id);
    if (auto it = m_textures.find(key); it != m_textures.end())
    {
        SDL_DestroyTexture(it->second);
        it->second = texture;
        return true;
    }

    m_textures.emplace(std::move(key), texture);
    return true;
}

SDL_Texture* TextureCache::get(std::string_view id) const
{
    auto it = m_textures.find(id);
    return it != m_textures.end() ? it->second : nullptr;
}

void TextureCache::draw(std::string_view id, const DrawParams& params) const
{
    SDL_Texture* texture = get(id);
    if (texture == nullptr)
    {
        return;
    }

    const SDL_Rect destination{
        params.position.x, params.position.y,
        static_cast<int>(static_cast<float>(params.source.w) * params.scale),
        static_cast<int>(static_cast<float>(params.source.h) * params.scale)};

    SDL_SetTextureAlphaMod(texture, params.alpha);
    SDL_RenderCopyEx(m_renderer, texture, &params.source, &destination,
                     params.angle, nullptr, params.flip);
}

void TextureCache::remove(std::string_view id)
{
    auto it = m_textures.find(id);
    if (it != m_textures.end())
    {
        SDL_DestroyTexture(it->second);
        m_textures.erase(it);
    }
}

void TextureCache::clear() { destroyAll(); }

} // namespace de

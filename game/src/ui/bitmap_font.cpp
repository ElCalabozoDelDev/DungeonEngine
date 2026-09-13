#include <SDL.h>
#include <SDL_image.h>
#include <cstdlib>
#include <fstream>
#include <game/ui/bitmap_font.hpp>
#include <iostream>
#include <string>

namespace game::ui
{
namespace
{
int readKeyInt(const std::string& line, const char* key, int fallback = 0)
{
    const std::string token = std::string(key) + "=";
    const auto pos = line.find(token);
    if (pos == std::string::npos)
    {
        return fallback;
    }
    return std::atoi(line.c_str() + pos + token.size());
}

std::string readKeyString(const std::string& line, const char* key)
{
    const std::string token = std::string(key) + "=\"";
    const auto pos = line.find(token);
    if (pos == std::string::npos)
    {
        return {};
    }
    const auto start = pos + token.size();
    const auto end = line.find('"', start);
    if (end == std::string::npos)
    {
        return {};
    }
    return line.substr(start, end - start);
}
} // namespace

bool BitmapFont::load(SDL_Renderer* renderer, const std::string& fntPath,
                      const std::string& pagePath)
{
    destroy();
    if (renderer == nullptr)
    {
        return false;
    }

    std::ifstream in(fntPath);
    if (!in)
    {
        std::cerr << "BitmapFont: cannot open '" << fntPath << "'\n";
        return false;
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (line.starts_with("common "))
        {
            m_lineHeight = readKeyInt(line, "lineHeight", 35);
            m_pageW = readKeyInt(line, "scaleW", 256);
            m_pageH = readKeyInt(line, "scaleH", 512);
        }
        else if (line.starts_with("char "))
        {
            Glyph g;
            const int id = readKeyInt(line, "id");
            g.x = readKeyInt(line, "x");
            g.y = readKeyInt(line, "y");
            g.width = readKeyInt(line, "width");
            g.height = readKeyInt(line, "height");
            g.xOffset = readKeyInt(line, "xoffset");
            g.yOffset = readKeyInt(line, "yoffset");
            g.xAdvance = readKeyInt(line, "xadvance");
            m_glyphs.emplace(id, g);
        }
    }

    SDL_Surface* surface = IMG_Load(pagePath.c_str());
    if (surface == nullptr)
    {
        // Fallback: SDL_image header via texture cache path — include here.
        std::cerr << "BitmapFont: cannot load page '" << pagePath
                  << "': " << IMG_GetError() << '\n';
        return false;
    }
    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (m_texture == nullptr)
    {
        std::cerr << "BitmapFont: CreateTexture failed: " << SDL_GetError()
                  << '\n';
        return false;
    }
    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(m_texture, SDL_ScaleModeNearest);

    // The .fnt common line often keeps the original BMFont page size
    // (scaleW=256) even after glyphs were pasted into a larger atlas.png
    // (512×512). UVs must use the real texture dimensions.
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(m_texture, nullptr, nullptr, &texW, &texH);
    if (texW > 0 && texH > 0)
    {
        m_pageW = texW;
        m_pageH = texH;
    }
    return true;
}

void BitmapFont::destroy()
{
    if (m_texture != nullptr)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
    m_glyphs.clear();
}

const BitmapFont::Glyph* BitmapFont::glyph(int codepoint) const
{
    auto it = m_glyphs.find(codepoint);
    return it != m_glyphs.end() ? &it->second : nullptr;
}

ImVec2 BitmapFont::measure(std::string_view text, float scale) const
{
    float w = 0.0f;
    for (unsigned char ch : text)
    {
        const Glyph* g = glyph(static_cast<int>(ch));
        if (g == nullptr)
        {
            continue;
        }
        w += static_cast<float>(g->xAdvance) * scale;
    }
    return ImVec2(w, static_cast<float>(m_lineHeight) * scale);
}

void BitmapFont::draw(ImDrawList* draw, ImVec2 pos, std::string_view text,
                      float scale, ImU32 color) const
{
    if (draw == nullptr || m_texture == nullptr || scale <= 0.0f)
    {
        return;
    }
    const ImTextureID id = reinterpret_cast<ImTextureID>(m_texture);
    const float invW = 1.0f / static_cast<float>(m_pageW);
    const float invH = 1.0f / static_cast<float>(m_pageH);
    float cursorX = pos.x;
    const float cursorY = pos.y;
    for (unsigned char ch : text)
    {
        const Glyph* g = glyph(static_cast<int>(ch));
        if (g == nullptr)
        {
            continue;
        }
        if (g->width > 0 && g->height > 0)
        {
            const ImVec2 p0(cursorX + static_cast<float>(g->xOffset) * scale,
                            cursorY + static_cast<float>(g->yOffset) * scale);
            const ImVec2 p1(p0.x + static_cast<float>(g->width) * scale,
                            p0.y + static_cast<float>(g->height) * scale);
            const ImVec2 uv0(static_cast<float>(g->x) * invW,
                             static_cast<float>(g->y) * invH);
            const ImVec2 uv1(static_cast<float>(g->x + g->width) * invW,
                             static_cast<float>(g->y + g->height) * invH);
            draw->AddImage(id, p0, p1, uv0, uv1, color);
        }
        cursorX += static_cast<float>(g->xAdvance) * scale;
    }
}

} // namespace game::ui

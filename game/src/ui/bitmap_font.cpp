#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <game/ui/bitmap_font.hpp>
#include <iostream>
#include <string>
#include <vector>

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

    SDL_Surface* src = IMG_Load(pagePath.c_str());
    if (src == nullptr)
    {
        std::cerr << "BitmapFont: cannot load page '" << pagePath
                  << "': " << IMG_GetError() << '\n';
        return false;
    }

    // Snapshot source rects, then pack into a padded private page so sampling
    // never bleeds slime/UI texels from the shared atlas.png.
    struct SrcGlyph
    {
        int srcX;
        int srcY;
        Glyph* glyph;
    };
    std::vector<SrcGlyph> items;
    items.reserve(m_glyphs.size());
    for (auto& [id, g] : m_glyphs)
    {
        (void)id;
        items.push_back({g.x, g.y, &g});
    }
    std::sort(items.begin(), items.end(),
              [](const SrcGlyph& a, const SrcGlyph& b)
              { return a.glyph->height > b.glyph->height; });

    constexpr int kPad = 1;
    constexpr int kAtlasW = 512;
    int cursorX = kPad;
    int cursorY = kPad;
    int rowH = 0;
    int packedW = kPad;
    int packedH = kPad;
    for (SrcGlyph& item : items)
    {
        if (item.glyph->width <= 0 || item.glyph->height <= 0)
        {
            item.glyph->x = 0;
            item.glyph->y = 0;
            continue;
        }
        const int cellW = item.glyph->width + kPad * 2;
        const int cellH = item.glyph->height + kPad * 2;
        if (cursorX + cellW > kAtlasW)
        {
            cursorX = kPad;
            cursorY += rowH;
            rowH = 0;
        }
        item.glyph->x = cursorX + kPad;
        item.glyph->y = cursorY + kPad;
        cursorX += cellW;
        rowH = std::max(rowH, cellH);
        packedW = std::max(packedW, cursorX);
        packedH = std::max(packedH, cursorY + rowH);
    }
    packedW = std::max(packedW + kPad, 1);
    packedH = std::max(packedH + kPad, 1);

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, packedW, packedH, 32,
                                                      SDL_PIXELFORMAT_RGBA32);
    if (dst == nullptr)
    {
        SDL_FreeSurface(src);
        std::cerr << "BitmapFont: CreateRGBSurface failed: " << SDL_GetError()
                  << '\n';
        return false;
    }
    SDL_FillRect(dst, nullptr, SDL_MapRGBA(dst->format, 0, 0, 0, 0));

    for (const SrcGlyph& item : items)
    {
        if (item.glyph->width <= 0 || item.glyph->height <= 0)
        {
            continue;
        }
        SDL_Rect srcRect{item.srcX, item.srcY, item.glyph->width,
                         item.glyph->height};
        SDL_Rect dstRect{item.glyph->x, item.glyph->y, item.glyph->width,
                         item.glyph->height};
        SDL_BlitSurface(src, &srcRect, dst, &dstRect);
    }
    SDL_FreeSurface(src);

    m_texture = SDL_CreateTextureFromSurface(renderer, dst);
    m_pageW = dst->w;
    m_pageH = dst->h;
    SDL_FreeSurface(dst);
    if (m_texture == nullptr)
    {
        std::cerr << "BitmapFont: CreateTexture failed: " << SDL_GetError()
                  << '\n';
        return false;
    }
    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(m_texture, SDL_ScaleModeNearest);
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
    // Integer logical origin; nearest sampling so upscales expand texels
    // instead of blending a soft fringe (no TTF antialias).
    pos.x = std::floor(pos.x);
    pos.y = std::floor(pos.y);
    SDL_SetTextureScaleMode(m_texture, SDL_ScaleModeNearest);
    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    const ImTextureID id = reinterpret_cast<ImTextureID>(m_texture);
    const float invW = 1.0f / static_cast<float>(m_pageW);
    const float invH = 1.0f / static_cast<float>(m_pageH);
    float cursorX = pos.x;
    const float cursorY = pos.y;
    const bool integerScale = std::floor(scale) == scale;
    for (unsigned char ch : text)
    {
        const Glyph* g = glyph(static_cast<int>(ch));
        if (g == nullptr)
        {
            continue;
        }
        if (g->width > 0 && g->height > 0)
        {
            float x = cursorX + static_cast<float>(g->xOffset) * scale;
            float y = cursorY + static_cast<float>(g->yOffset) * scale;
            float w = static_cast<float>(g->width) * scale;
            float h = static_cast<float>(g->height) * scale;
            if (integerScale)
            {
                x = std::floor(x);
                y = std::floor(y);
                w = std::floor(w);
                h = std::floor(h);
            }
            const ImVec2 p0(x, y);
            const ImVec2 p1(x + w, y + h);
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

#ifndef GAME_UI_BITMAP_FONT_HPP
#define GAME_UI_BITMAP_FONT_HPP

#include <SDL.h>
#include <imgui.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace game::ui
{
/// AngelCode BMFont (.fnt) + atlas page — same path Gum uses in the tutorial.
class BitmapFont
{
public:
    struct Glyph
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        int xOffset = 0;
        int yOffset = 0;
        int xAdvance = 0;
    };

    BitmapFont() = default;
    ~BitmapFont() { destroy(); }

    BitmapFont(const BitmapFont&) = delete;
    BitmapFont& operator=(const BitmapFont&) = delete;

    BitmapFont(BitmapFont&& other) noexcept
        : m_texture(other.m_texture), m_lineHeight(other.m_lineHeight),
          m_pageW(other.m_pageW), m_pageH(other.m_pageH),
          m_glyphs(std::move(other.m_glyphs))
    {
        other.m_texture = nullptr;
    }

    BitmapFont& operator=(BitmapFont&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            m_texture = other.m_texture;
            m_lineHeight = other.m_lineHeight;
            m_pageW = other.m_pageW;
            m_pageH = other.m_pageH;
            m_glyphs = std::move(other.m_glyphs);
            other.m_texture = nullptr;
        }
        return *this;
    }

    bool load(SDL_Renderer* renderer, const std::string& fntPath,
              const std::string& pagePath);
    void destroy();

    bool ok() const { return m_texture != nullptr; }
    SDL_Texture* texture() const { return m_texture; }
    int lineHeight() const { return m_lineHeight; }

    const Glyph* glyph(int codepoint) const;
    ImVec2 measure(std::string_view text, float scale) const;
    void draw(ImDrawList* draw, ImVec2 pos, std::string_view text, float scale,
              ImU32 color) const;

private:
    SDL_Texture* m_texture = nullptr;
    int m_lineHeight = 0;
    int m_pageW = 1;
    int m_pageH = 1;
    std::unordered_map<int, Glyph> m_glyphs;
};

} // namespace game::ui

#endif // GAME_UI_BITMAP_FONT_HPP

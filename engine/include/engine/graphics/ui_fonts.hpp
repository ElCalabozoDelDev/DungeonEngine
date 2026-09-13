#ifndef DE_GRAPHICS_UI_FONTS_HPP
#define DE_GRAPHICS_UI_FONTS_HPP

struct ImFont;

namespace de
{
/// ImGui fonts baked at startup (debug / fallback). Game UI and the title
/// screen use the AngelCode BMFont with nearest sampling instead.
struct UiFonts
{
    ImFont* body = nullptr;
};

} // namespace de

#endif // DE_GRAPHICS_UI_FONTS_HPP

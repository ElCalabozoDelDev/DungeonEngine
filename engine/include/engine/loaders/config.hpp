#ifndef DE_LOADERS_CONFIG_HPP
#define DE_LOADERS_CONFIG_HPP

#include <map>
#include <string>

namespace de
{
/// Startup settings, read from assets/game.json.
///
/// Every member has a default. The previous version had none, so a failed
/// parse left the whole struct indeterminate and `1000 / frameRate` in
/// SDLPlugin divided by garbage.
struct Config
{
    std::string title = "Dungeon Slime";
    bool fullScreen = false;
    int screenWidth = 1280;
    int screenHeight = 720;
    int frameRate = 60;

    /// When true the renderer presents with vsync and frameRate only acts as
    /// a hint; when false the loop paces itself to frameRate with SDL_Delay.
    bool vsync = true;
    bool debug = false;

    /// Logical render size (SDL_RenderSetLogicalSize / fixed camera defaults),
    /// not a gameplay follow-camera. Mapped from game.json's logicalWidth and
    /// logicalHeight so SDLPlugin keeps working without a rename.
    float cameraWidth = 320.0f;
    float cameraHeight = 180.0f;
    float zoomLevel = 1.0f;

    std::map<std::string, std::string> levels;
};

} // namespace de

#endif // DE_LOADERS_CONFIG_HPP

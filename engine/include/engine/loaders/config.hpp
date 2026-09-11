#ifndef DE_LOADERS_CONFIG_HPP
#define DE_LOADERS_CONFIG_HPP

#include <map>
#include <string>

namespace de
{
/// Startup settings, read from assets/game.xml.
///
/// Every member has a default. The previous version had none, so a failed
/// parse left the whole struct indeterminate and `1000 / frameRate` in
/// SDLPlugin divided by garbage.
struct Config
{
    std::string title = "DungeonEngine";
    bool fullScreen = false;
    int screenWidth = 800;
    int screenHeight = 600;
    int frameRate = 60;

    /// When true the renderer presents with vsync and frameRate only acts as
    /// a hint; when false the loop paces itself to frameRate with SDL_Delay.
    bool vsync = true;
    bool debug = false;
    /// Logical render size: the coordinate space the game draws in, scaled
    /// by SDL to fit the window.
    float cameraWidth = 800.0f;
    float cameraHeight = 600.0f;
    float zoomLevel = 1.0f;
    std::map<std::string, std::string> levels;
};

} // namespace de

#endif // DE_LOADERS_CONFIG_HPP

#ifndef DE_LOADERS_CONFIG_HPP
#define DE_LOADERS_CONFIG_HPP
#include <map>
#include <string>

namespace de
{
struct Config
{
    std::string title;
    bool fullScreen;
    int screenWidth;
    int screenHeight;
    int frameRate;
    bool debug;
    float cameraWidth;
    float cameraHeight;
    float viewportOffsetX;
    float viewportOffsetY;
    float zoomLevel;
    std::map<std::string, std::string> levels;
};

} // namespace de

#endif // DE_LOADERS_CONFIG_HPP

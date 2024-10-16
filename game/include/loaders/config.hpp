#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <map>
#include <string>

struct Config {
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
    std::map<std::string, std::string> levels;
};

#endif
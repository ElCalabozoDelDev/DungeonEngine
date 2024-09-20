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
    std::map<std::string, std::string> levels;
};

#endif
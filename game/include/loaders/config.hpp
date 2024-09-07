#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>

struct Config {
    std::string title;
    bool fullScreen;
    int screenWidth;
    int screenHeight;
    int frameRate;
};

#endif
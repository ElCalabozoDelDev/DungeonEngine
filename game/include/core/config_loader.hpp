#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include "config.hpp"

class ConfigLoader {
    public:
        ConfigLoader();
        ~ConfigLoader();
        static bool loadConfigFromXML(const char* path, Config& config);
};

#endif
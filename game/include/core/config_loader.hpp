#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

class ConfigLoader {
    public:
        ConfigLoader();
        ~ConfigLoader();
        static bool loadConfigFromXML(const char* path);
};

#endif
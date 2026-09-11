#ifndef DE_LOADERS_CONFIG_LOADER_HPP
#define DE_LOADERS_CONFIG_LOADER_HPP

#include <engine/loaders/config.hpp>

namespace de
{
class ConfigLoader
{
public:
    ConfigLoader() = delete;

    static bool loadConfigFromXML(const char* path, Config& config);
};

} // namespace de

#endif // DE_LOADERS_CONFIG_LOADER_HPP

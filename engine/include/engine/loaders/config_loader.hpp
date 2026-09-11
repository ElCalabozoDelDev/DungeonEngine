#ifndef DE_LOADERS_CONFIG_LOADER_HPP
#define DE_LOADERS_CONFIG_LOADER_HPP

#include <engine/loaders/config.hpp>
#include <expected>
#include <filesystem>
#include <string>

namespace de
{
class ConfigLoader
{
public:
    ConfigLoader() = delete;

    /// Reads the game configuration from `path`.
    ///
    /// Returns a message describing what was wrong instead of leaving a
    /// half-filled Config behind: the old signature returned a bool that
    /// BasePlugin discarded, and reached into
    /// Game->Config->Init through three unchecked pointers.
    static std::expected<Config, std::string>
    load(const std::filesystem::path& path);
};

} // namespace de

#endif // DE_LOADERS_CONFIG_LOADER_HPP

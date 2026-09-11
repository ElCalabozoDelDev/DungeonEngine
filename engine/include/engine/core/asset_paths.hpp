#ifndef DE_CORE_ASSET_PATHS_HPP
#define DE_CORE_ASSET_PATHS_HPP

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace de
{
/// Resolves asset files relative to the assets directory, found once at
/// startup from the executable's own location.
///
/// Paths used to be hardcoded relative to the working directory
/// ("../assets/game.xml"), so the game only ran when launched from
/// build/bin and double-clicking the executable failed.
class AssetPaths
{
public:
    explicit AssetPaths(std::filesystem::path root) noexcept
        : m_root(std::move(root))
    {
    }

    /// Looks for `dirName` next to the executable, then one and two levels
    /// above it, which covers both an installed layout (assets beside the
    /// binary) and this build tree (bin/ and assets/ as siblings).
    static std::expected<AssetPaths, std::string>
    discover(std::string_view dirName = "assets");

    const std::filesystem::path& root() const noexcept { return m_root; }

    /// Resolves `relative` against the assets root. An absolute path is
    /// returned unchanged.
    std::filesystem::path resolve(std::string_view relative) const;

private:
    std::filesystem::path m_root;
};

} // namespace de

#endif // DE_CORE_ASSET_PATHS_HPP

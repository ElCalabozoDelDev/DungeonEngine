#include <SDL.h>
#include <array>
#include <engine/core/asset_paths.hpp>

namespace de
{
std::expected<AssetPaths, std::string>
AssetPaths::discover(std::string_view dirName)
{
    char* base = SDL_GetBasePath();
    if (base == nullptr)
    {
        return std::unexpected(
            std::string("could not locate the executable directory: ") +
            SDL_GetError());
    }

    const std::filesystem::path basePath(base);
    SDL_free(base);

    const std::array<std::filesystem::path, 3> candidates{
        basePath / dirName,
        basePath.parent_path() / dirName,
        basePath.parent_path().parent_path() / dirName,
    };

    std::error_code ec;
    for (const auto& candidate : candidates)
    {
        if (std::filesystem::is_directory(candidate, ec))
        {
            return AssetPaths(std::filesystem::weakly_canonical(candidate, ec));
        }
    }

    std::string tried;
    for (const auto& candidate : candidates)
    {
        tried += "\n  " + candidate.string();
    }
    return std::unexpected("could not find an '" + std::string(dirName) +
                           "' directory. Looked in:" + tried);
}

std::filesystem::path AssetPaths::resolve(std::string_view relative) const
{
    std::filesystem::path path(relative);
    if (path.is_absolute())
    {
        return path;
    }
    return (m_root / path).lexically_normal();
}

} // namespace de

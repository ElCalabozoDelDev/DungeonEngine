#include <engine/loaders/config_loader.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

namespace de
{
namespace
{
using TypeCheck = bool (nlohmann::json::*)() const noexcept;

/// Copies `root[key]` into `out` when it is present and passes `isType`;
/// otherwise `out` keeps its default. A value of the wrong type is ignored
/// rather than rejected, as a missing one is.
template <typename T>
void readOptional(const nlohmann::json& root, const char* key, TypeCheck isType,
                  T& out)
{
    if (auto it = root.find(key); it != root.end() && ((*it).*isType)())
    {
        out = it->get<T>();
    }
}

} // namespace

std::expected<Config, std::string>
ConfigLoader::load(const std::filesystem::path& path)
{
    std::ifstream in(path);
    if (!in)
    {
        return std::unexpected("could not read '" + path.string() + "'");
    }

    nlohmann::json root;
    try
    {
        in >> root;
    }
    catch (const nlohmann::json::exception& ex)
    {
        return std::unexpected("could not parse '" + path.string() +
                               "': " + ex.what());
    }

    if (!root.is_object())
    {
        return std::unexpected("config root must be a JSON object");
    }

    using json = nlohmann::json;
    Config result;
    readOptional(root, "title", &json::is_string, result.title);
    readOptional(root, "fullScreen", &json::is_boolean, result.fullScreen);
    readOptional(root, "screenWidth", &json::is_number_integer,
                 result.screenWidth);
    readOptional(root, "screenHeight", &json::is_number_integer,
                 result.screenHeight);
    readOptional(root, "frameRate", &json::is_number_integer, result.frameRate);
    readOptional(root, "vsync", &json::is_boolean, result.vsync);
    readOptional(root, "debug", &json::is_boolean, result.debug);
    readOptional(root, "logicalWidth", &json::is_number, result.logicalWidth);
    readOptional(root, "logicalHeight", &json::is_number, result.logicalHeight);
    readOptional(root, "zoomLevel", &json::is_number, result.zoomLevel);

    if (root.contains("levels"))
    {
        if (!root["levels"].is_object())
        {
            return std::unexpected("'levels' must be a JSON object");
        }
        for (auto it = root["levels"].begin(); it != root["levels"].end(); ++it)
        {
            if (!it.value().is_string())
            {
                return std::unexpected(
                    "every levels entry must be a string path");
            }
            result.levels[it.key()] = it.value().get<std::string>();
        }
    }

    if (result.frameRate <= 0)
    {
        return std::unexpected("frameRate must be greater than zero, got " +
                               std::to_string(result.frameRate));
    }
    if (result.zoomLevel <= 0.0f)
    {
        return std::unexpected("zoomLevel must be greater than zero");
    }
    if (result.levels.empty())
    {
        return std::unexpected("'levels' contains no entries");
    }

    return result;
}

} // namespace de

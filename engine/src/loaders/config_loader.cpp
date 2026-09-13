#include <engine/loaders/config_loader.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

namespace de
{
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

    Config result;

    if (root.contains("title") && root["title"].is_string())
    {
        result.title = root["title"].get<std::string>();
    }
    if (root.contains("fullScreen") && root["fullScreen"].is_boolean())
    {
        result.fullScreen = root["fullScreen"].get<bool>();
    }
    if (root.contains("screenWidth") && root["screenWidth"].is_number_integer())
    {
        result.screenWidth = root["screenWidth"].get<int>();
    }
    if (root.contains("screenHeight") &&
        root["screenHeight"].is_number_integer())
    {
        result.screenHeight = root["screenHeight"].get<int>();
    }
    if (root.contains("frameRate") && root["frameRate"].is_number_integer())
    {
        result.frameRate = root["frameRate"].get<int>();
    }
    if (root.contains("vsync") && root["vsync"].is_boolean())
    {
        result.vsync = root["vsync"].get<bool>();
    }
    if (root.contains("debug") && root["debug"].is_boolean())
    {
        result.debug = root["debug"].get<bool>();
    }

    // logicalWidth/Height map onto cameraWidth/Height for SDL logical size.
    if (root.contains("logicalWidth") && root["logicalWidth"].is_number())
    {
        result.cameraWidth = root["logicalWidth"].get<float>();
    }
    if (root.contains("logicalHeight") && root["logicalHeight"].is_number())
    {
        result.cameraHeight = root["logicalHeight"].get<float>();
    }
    if (root.contains("zoomLevel") && root["zoomLevel"].is_number())
    {
        result.zoomLevel = root["zoomLevel"].get<float>();
    }

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

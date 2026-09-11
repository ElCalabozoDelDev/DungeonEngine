#include <engine/loaders/config_loader.hpp>
#include <tinyxml2.h>

namespace de
{
namespace
{
using tinyxml2::XMLElement;

/// Fetches a required child element, reporting which one was missing rather
/// than dereferencing a null pointer the way the old chained calls did.
std::expected<XMLElement*, std::string> child(XMLElement* parent,
                                              const char* name)
{
    XMLElement* element = parent->FirstChildElement(name);
    if (element == nullptr)
    {
        return std::unexpected(std::string("missing <") + name + "> element");
    }
    return element;
}

} // namespace

std::expected<Config, std::string>
ConfigLoader::load(const std::filesystem::path& path)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path.string().c_str()) != tinyxml2::XML_SUCCESS)
    {
        return std::unexpected("could not read '" + path.string() +
                               "': " + doc.ErrorStr());
    }

    XMLElement* game = doc.FirstChildElement("Game");
    if (game == nullptr)
    {
        return std::unexpected("missing <Game> root element");
    }

    auto config = child(game, "Config");
    if (!config)
    {
        return std::unexpected(config.error());
    }
    auto init = child(*config, "Init");
    if (!init)
    {
        return std::unexpected(init.error());
    }
    XMLElement* root = *init;

    Config result;

    auto title = child(root, "Title");
    if (!title)
    {
        return std::unexpected(title.error());
    }
    const char* titleText = (*title)->GetText();
    result.title = titleText != nullptr ? titleText : result.title;

    auto fullScreen = child(root, "FullScreen");
    if (!fullScreen)
    {
        return std::unexpected(fullScreen.error());
    }
    result.fullScreen = (*fullScreen)->BoolText(result.fullScreen);

    auto frameRate = child(root, "FrameRate");
    if (!frameRate)
    {
        return std::unexpected(frameRate.error());
    }
    result.frameRate = (*frameRate)->IntText(result.frameRate);

    auto debug = child(root, "Debug");
    if (!debug)
    {
        return std::unexpected(debug.error());
    }
    result.debug = (*debug)->BoolText(result.debug);

    auto screen = child(root, "Screen");
    if (!screen)
    {
        return std::unexpected(screen.error());
    }
    // Optional: absent means keep the default (on).
    result.vsync = (*screen)->BoolAttribute("vsync", result.vsync);
    result.screenWidth = (*screen)->IntAttribute("width", result.screenWidth);
    result.screenHeight =
        (*screen)->IntAttribute("height", result.screenHeight);

    auto camera = child(root, "Camera");
    if (!camera)
    {
        return std::unexpected(camera.error());
    }
    result.cameraWidth = (*camera)->FloatAttribute("width", result.cameraWidth);
    result.cameraHeight =
        (*camera)->FloatAttribute("height", result.cameraHeight);
    result.zoomLevel = (*camera)->FloatAttribute("zoomLevel", result.zoomLevel);

    auto levels = child(root, "Levels");
    if (!levels)
    {
        return std::unexpected(levels.error());
    }
    for (XMLElement* level = (*levels)->FirstChildElement("Level");
         level != nullptr; level = level->NextSiblingElement("Level"))
    {
        const char* name = level->Attribute("name");
        const char* levelPath = level->Attribute("path");
        if (name == nullptr || levelPath == nullptr)
        {
            return std::unexpected(
                "every <Level> needs both a name and a path attribute");
        }
        result.levels[name] = levelPath;
    }

    // Values the rest of the engine divides by, checked once here instead of
    // failing far away: frameRate feeds `1000 / frameRate` in SDLPlugin and
    // zoomLevel divides the camera extents in RenderSystem.
    if (result.frameRate <= 0)
    {
        return std::unexpected("<FrameRate> must be greater than zero, got " +
                               std::to_string(result.frameRate));
    }
    if (result.zoomLevel <= 0.0f)
    {
        return std::unexpected("<Camera zoomLevel> must be greater than zero");
    }
    if (result.levels.empty())
    {
        return std::unexpected("<Levels> contains no <Level> entries");
    }

    return result;
}

} // namespace de

#include <doctest/doctest.h>
#include <engine/loaders/config_loader.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace de;

namespace
{
/// Writes `contents` to a uniquely named temp file and removes it again.
class TempFile
{
public:
    explicit TempFile(const std::string& contents)
    {
        static int counter = 0;
        m_path = std::filesystem::temp_directory_path() /
                 ("de_test_" + std::to_string(++counter) + ".xml");
        std::ofstream out(m_path);
        out << contents;
    }

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(m_path, ec);
    }

    const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

const char* ValidConfig =
    "<?xml version=\"1.0\"?>\n"
    "<Game><Config><Init>\n"
    "  <Title>Test</Title>\n"
    "  <FullScreen>false</FullScreen>\n"
    "  <FrameRate>60</FrameRate>\n"
    "  <Screen width=\"1024\" height=\"768\" vsync=\"false\" />\n"
    "  <Debug>true</Debug>\n"
    "  <Camera width=\"640\" height=\"480\" zoomLevel=\"2\" />\n"
    "  <Levels><Level name=\"level1\" path=\"Levels/a.tmx\" /></Levels>\n"
    "</Init></Config></Game>\n";

/// The valid config with one element's line removed.
std::string without(const std::string& element)
{
    std::string xml = ValidConfig;
    const auto start = xml.find("<" + element);
    REQUIRE(start != std::string::npos);
    const auto end = xml.find('\n', start);
    xml.erase(start, end - start);
    return xml;
}

std::string replacing(const std::string& from, const std::string& to)
{
    std::string xml = ValidConfig;
    const auto at = xml.find(from);
    REQUIRE(at != std::string::npos);
    xml.replace(at, from.size(), to);
    return xml;
}

} // namespace

TEST_CASE("a valid config parses")
{
    TempFile file(ValidConfig);
    auto config = ConfigLoader::load(file.path());

    REQUIRE(config.has_value());
    CHECK(config->title == "Test");
    CHECK(config->fullScreen == false);
    CHECK(config->frameRate == 60);
    CHECK(config->screenWidth == 1024);
    CHECK(config->screenHeight == 768);
    CHECK(config->vsync == false); // optional attribute, present
    CHECK(config->debug == true);
    CHECK(config->cameraWidth == doctest::Approx(640.0f));
    CHECK(config->cameraHeight == doctest::Approx(480.0f));
    CHECK(config->zoomLevel == doctest::Approx(2.0f));
    CHECK(config->levels.at("level1") == "Levels/a.tmx");
}

TEST_CASE("vsync defaults to on when the attribute is absent")
{
    TempFile file(replacing(" vsync=\"false\"", ""));
    auto config = ConfigLoader::load(file.path());
    REQUIRE(config.has_value());
    CHECK(config->vsync == true);
}

TEST_CASE("a missing file is reported, not crashed on")
{
    auto config = ConfigLoader::load("no/such/file.xml");
    REQUIRE_FALSE(config.has_value());
    CHECK(config.error().find("could not read") != std::string::npos);
}

TEST_CASE("malformed XML is reported")
{
    TempFile file("<Game><Config><Init> never closed");
    auto config = ConfigLoader::load(file.path());
    REQUIRE_FALSE(config.has_value());
}

TEST_CASE("a missing element is named")
{
    // The loader used to chain FirstChildElement calls without checking, so a
    // missing <Game> dereferenced null before reaching its own null check.
    for (const std::string element : {"Title", "FullScreen", "FrameRate",
                                      "Screen", "Debug", "Camera", "Levels"})
    {
        CAPTURE(element);
        TempFile file(without(element));
        auto config = ConfigLoader::load(file.path());
        REQUIRE_FALSE(config.has_value());
        CHECK(config.error().find(element) != std::string::npos);
    }
}

TEST_CASE("a missing root element is reported")
{
    TempFile file("<?xml version=\"1.0\"?><NotAGame/>");
    auto config = ConfigLoader::load(file.path());
    REQUIRE_FALSE(config.has_value());
    CHECK(config.error().find("Game") != std::string::npos);
}

TEST_CASE("values the engine divides by are rejected")
{
    SUBCASE("a frame rate of zero")
    {
        // This fed `1000 / frameRate` in SDLPlugin.
        TempFile file(replacing("<FrameRate>60", "<FrameRate>0"));
        auto config = ConfigLoader::load(file.path());
        REQUIRE_FALSE(config.has_value());
        CHECK(config.error().find("FrameRate") != std::string::npos);
    }

    SUBCASE("a zoom of zero")
    {
        // This divides the camera extents in Camera2D.
        TempFile file(replacing("zoomLevel=\"2\"", "zoomLevel=\"0\""));
        auto config = ConfigLoader::load(file.path());
        REQUIRE_FALSE(config.has_value());
        CHECK(config.error().find("zoomLevel") != std::string::npos);
    }
}

TEST_CASE("an empty level list is rejected")
{
    TempFile file(replacing(
        "<Levels><Level name=\"level1\" path=\"Levels/a.tmx\" /></Levels>",
        "<Levels></Levels>"));
    auto config = ConfigLoader::load(file.path());
    REQUIRE_FALSE(config.has_value());
    CHECK(config.error().find("Level") != std::string::npos);
}

#include <doctest/doctest.h>
#include <engine/loaders/config_loader.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace de;

namespace
{
class TempFile
{
public:
    explicit TempFile(const std::string& contents,
                      const std::string& extension = ".json")
    {
        static int counter = 0;
        m_path = std::filesystem::temp_directory_path() /
                 ("de_test_" + std::to_string(++counter) + extension);
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

const char* ValidConfig = R"({
  "title": "Dungeon Slime",
  "fullScreen": false,
  "screenWidth": 1280,
  "screenHeight": 720,
  "frameRate": 60,
  "vsync": true,
  "debug": false,
  "logicalWidth": 320,
  "logicalHeight": 180,
  "levels": { "arena": "tilemap.tmj" }
})";

} // namespace

TEST_CASE("a valid game.json parses")
{
    TempFile file(ValidConfig);
    auto config = ConfigLoader::load(file.path());

    REQUIRE(config.has_value());
    CHECK(config->title == "Dungeon Slime");
    CHECK(config->fullScreen == false);
    CHECK(config->frameRate == 60);
    CHECK(config->screenWidth == 1280);
    CHECK(config->screenHeight == 720);
    CHECK(config->vsync == true);
    CHECK(config->debug == false);
    CHECK(config->logicalWidth == doctest::Approx(320.0f));
    CHECK(config->logicalHeight == doctest::Approx(180.0f));
    CHECK(config->zoomLevel == doctest::Approx(1.0f));
    CHECK(config->levels.at("arena") == "tilemap.tmj");
}

TEST_CASE("vsync defaults to on when absent")
{
    TempFile file(R"({
      "title": "T",
      "frameRate": 60,
      "logicalWidth": 320,
      "logicalHeight": 180,
      "levels": { "arena": "a.tmj" }
    })");
    auto config = ConfigLoader::load(file.path());
    REQUIRE(config.has_value());
    CHECK(config->vsync == true);
}

TEST_CASE("a missing file is reported, not crashed on")
{
    auto config = ConfigLoader::load("no/such/file.json");
    REQUIRE_FALSE(config.has_value());
}

TEST_CASE("empty levels is rejected")
{
    TempFile file(R"({
      "title": "T",
      "frameRate": 60,
      "levels": {}
    })");
    auto config = ConfigLoader::load(file.path());
    REQUIRE_FALSE(config.has_value());
}

TEST_CASE("zero frameRate is rejected")
{
    TempFile file(R"({
      "title": "T",
      "frameRate": 0,
      "levels": { "arena": "a.tmj" }
    })");
    auto config = ConfigLoader::load(file.path());
    REQUIRE_FALSE(config.has_value());
}

TEST_CASE("assets/game.json loads")
{
    auto config =
        ConfigLoader::load(std::filesystem::path(DE_ASSETS_DIR) / "game.json");
    REQUIRE(config.has_value());
    CHECK(config->title == "Dungeon Slime");
    CHECK(config->levels.count("arena") == 1);
}

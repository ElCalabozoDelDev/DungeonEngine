#include <doctest/doctest.h>
#include <game/command_line.hpp>
#include <initializer_list>
#include <vector>

namespace
{
std::expected<CommandLine, std::string>
parse(std::initializer_list<const char*> arguments)
{
    const std::vector<const char*> args(arguments);
    return parseCommandLine(args);
}

} // namespace

TEST_CASE("no arguments starts the game normally")
{
    const auto options = parse({});
    REQUIRE(options.has_value());
    CHECK(options->frames == 0);
    CHECK_FALSE(options->skipMenu);
    CHECK_FALSE(options->sim);
}

TEST_CASE("every option is read")
{
    const auto options = parse({"--frames", "30", "--level", "--sim",
                                "--sim-out", "run.csv", "--sim-steps", "120",
                                "--sim-seed", "4000000000", "--sim-window"});
    REQUIRE(options.has_value());
    CHECK(options->frames == 30);
    CHECK(options->skipMenu);
    CHECK(options->sim);
    CHECK(options->simOptions.csvPath == "run.csv");
    CHECK(options->simOptions.maxSteps == 120);
    CHECK(options->simOptions.seed == 4000000000u);
    CHECK(options->simOptions.window);
}

TEST_CASE("--sim starts in the level")
{
    const auto options = parse({"--sim"});
    REQUIRE(options.has_value());
    CHECK(options->skipMenu);
}

TEST_CASE("bad command lines are reported, not guessed at")
{
    // atoi turned each of these into 0 -- a frame limit of 0 runs forever,
    // a sim of 0 steps ends at once -- and an unknown option was only warned
    // about.
    SUBCASE("a number that is not one")
    {
        const auto options = parse({"--frames", "abc"});
        REQUIRE_FALSE(options.has_value());
        CHECK(options.error() == "invalid value for --frames: 'abc'");
    }

    SUBCASE("trailing junk")
    {
        CHECK_FALSE(parse({"--sim-steps", "12x"}).has_value());
    }

    SUBCASE("out of range")
    {
        CHECK_FALSE(parse({"--sim-steps", "0"}).has_value());
        CHECK_FALSE(parse({"--frames", "-5"}).has_value());
        CHECK_FALSE(parse({"--sim-seed", "-1"}).has_value());
        CHECK_FALSE(parse({"--frames", "99999999999"}).has_value());
    }

    SUBCASE("a missing value")
    {
        const auto options = parse({"--sim", "--sim-out"});
        REQUIRE_FALSE(options.has_value());
        CHECK(options.error() == "--sim-out needs a value");
    }

    SUBCASE("an unknown option")
    {
        const auto options = parse({"--fullscreen"});
        REQUIRE_FALSE(options.has_value());
        CHECK(options.error() == "unknown option: --fullscreen");
    }
}

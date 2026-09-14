#ifndef GAME_COMMAND_LINE_HPP
#define GAME_COMMAND_LINE_HPP

#include <expected>
#include <game/sim/sim_options.hpp>
#include <span>
#include <string>
#include <string_view>

/// What the command line asked for.
struct CommandLine
{
    /// Exit after this many frames; 0 runs until the window closes.
    int frames = 0;
    /// Start in the level instead of the title screen. Implied by --sim.
    bool skipMenu = false;
    bool sim = false;
    SimOptions simOptions;
};

inline constexpr std::string_view CommandLineUsage =
    "Usage: DungeonEngine [--frames N] [--level]\n"
    "                     [--sim [--sim-out PATH] [--sim-steps N]\n"
    "                            [--sim-seed N] [--sim-window]]\n";

/// Parses the arguments after the program name.
///
/// Fails, naming the problem, on an unknown option, a missing value, or a
/// number that is not one: `--frames abc` used to become 0 through atoi and
/// run forever instead.
std::expected<CommandLine, std::string>
parseCommandLine(std::span<const char* const> arguments);

#endif // GAME_COMMAND_LINE_HPP

#include <charconv>
#include <game/command_line.hpp>
#include <limits>
#include <optional>

namespace
{
/// The whole of `text` as a number in [min, max], or nothing.
template <typename T>
std::optional<T> parseNumber(std::string_view text, T min, T max)
{
    T value{};
    const auto* end = text.data() + text.size();
    const auto [ptr, ec] = std::from_chars(text.data(), end, value);
    if (ec != std::errc{} || ptr != end || value < min || value > max)
    {
        return std::nullopt;
    }
    return value;
}

} // namespace

std::expected<CommandLine, std::string>
parseCommandLine(std::span<const char* const> arguments)
{
    CommandLine result;

    for (std::size_t i = 0; i < arguments.size(); ++i)
    {
        const std::string_view option = arguments[i];

        // The value following an option that takes one.
        const auto value = [&]() -> std::optional<std::string_view>
        {
            if (i + 1 >= arguments.size())
            {
                return std::nullopt;
            }
            return std::string_view(arguments[++i]);
        };
        const auto number =
            [&]<typename T>(T min, T max) -> std::expected<T, std::string>
        {
            const auto text = value();
            if (!text)
            {
                return std::unexpected(std::string(option) + " needs a value");
            }
            if (auto parsed = parseNumber<T>(*text, min, max))
            {
                return *parsed;
            }
            return std::unexpected("invalid value for " + std::string(option) +
                                   ": '" + std::string(*text) + "'");
        };

        if (option == "--frames")
        {
            auto frames = number(0, std::numeric_limits<int>::max());
            if (!frames)
            {
                return std::unexpected(frames.error());
            }
            result.frames = *frames;
        }
        else if (option == "--level")
        {
            result.skipMenu = true;
        }
        else if (option == "--sim")
        {
            result.sim = true;
        }
        else if (option == "--sim-out")
        {
            const auto path = value();
            if (!path)
            {
                return std::unexpected("--sim-out needs a value");
            }
            result.simOptions.csvPath = std::string(*path);
        }
        else if (option == "--sim-steps")
        {
            auto steps = number(1, std::numeric_limits<int>::max());
            if (!steps)
            {
                return std::unexpected(steps.error());
            }
            result.simOptions.maxSteps = *steps;
        }
        else if (option == "--sim-seed")
        {
            auto seed = number(0u, std::numeric_limits<unsigned int>::max());
            if (!seed)
            {
                return std::unexpected(seed.error());
            }
            result.simOptions.seed = *seed;
        }
        else if (option == "--sim-window")
        {
            result.simOptions.window = true;
        }
        else
        {
            return std::unexpected("unknown option: " + std::string(option));
        }
    }

    if (result.sim)
    {
        result.skipMenu = true;
    }
    return result;
}
